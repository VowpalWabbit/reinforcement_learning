#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include "model_mgmt.h"
#include <boost/test/unit_test.hpp>

#include "api_status.h"
#include "config_utility.h"
#include "configuration.h"
#include "constants.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "model_mgmt/data_callback_fn.h"
#include "model_mgmt/model_downloader.h"
#include "object_factory.h"
#include "utility/periodic_background_proc.h"
#include "utility/watchdog.h"

#include <regex>
#include <unordered_map>

#ifdef USE_AZURE_FACTORIES
#  include "mock_http_client.h"
#  include "model_mgmt/restapi_data_transport.h"
#endif

namespace r = reinforcement_learning;
namespace m = reinforcement_learning::model_management;
namespace u = reinforcement_learning::utility;
namespace e = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;

const auto JSON_CFG = R"(
  {
    "ApplicationID": "rnc-123456-a",
    "EventHubInteractionConnectionString": "Endpoint=sb://localhost:8080/;SharedAccessKeyName=RMSAKey;SharedAccessKey=<ASharedAccessKey>=;EntityPath=interaction",
    "EventHubObservationConnectionString": "Endpoint=sb://localhost:8080/;SharedAccessKeyName=RMSAKey;SharedAccessKey=<ASharedAccessKey>=;EntityPath=observation",
    "IsExplorationEnabled": true,
    "ModelBlobUri": "http://localhost:8080",
    "InitialExplorationEpsilon": 1.0
  }
  )";
const auto JSON_CONTEXT = R"({"_multi":[{},{}]})";

m::model_data get_model_data();

int get_export_frequency(const u::configuration& cc, int& interval_ms, r::api_status* status)
{
  const auto export_freq_s = cc.get("ModelExportFrequency", nullptr);
  if (export_freq_s == nullptr) { RETURN_ERROR_LS(nullptr, status, model_export_frequency_not_provided); }
  // hh:mm:ss
  const std::regex re("\\s*([0-9]+):([0-9]+):([0-9]+)\\s*");
  std::cmatch m;
  if (std::regex_match(export_freq_s, m, re))
  {
    const auto hrs = atoi(m[1].str().c_str());
    const auto mins = atoi(m[2].str().c_str());
    const auto secs = atoi(m[3].str().c_str());
    interval_ms = hrs * 60 * 60 * 1000 + mins * 60 * 1000 + secs * 1000;
    if (interval_ms == 0) { RETURN_ERROR_LS(nullptr, status, bad_time_interval); }
    return e::success;
  }
  else { RETURN_ERROR_LS(nullptr, status, bad_time_interval); }
}

void dummy_error_fn(const r::api_status& err, void* ctxt) { *((int*)ctxt) = 10; }

void dummy_data_fn(const m::model_data& data, int* ctxt) { *ctxt = 20; }

#ifdef _WIN32  //_WIN32 (http_server http protocol issues in linux)
#  ifdef USE_AZURE_FACTORIES
BOOST_AUTO_TEST_CASE(background_mock_azure_get)
{
  // create a simple ds configuration
  u::configuration cc;
  auto scode = cfg::create_from_json(JSON_CFG, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  cc.set(r::name::EH_TEST, "true");  // local test event hub
  cc.set("ModelExportFrequency", "00:01:00");

  auto http_client = new mock_http_client("http://test.com");
  std::unique_ptr<m::i_data_transport> transport(new m::restapi_data_transport(http_client, nullptr));

  r::api_status status;

  int repeatms;
  get_export_frequency(cc, repeatms, &status);

  int err_ctxt;
  int data_ctxt;
  r::error_callback_fn efn(dummy_error_fn, &err_ctxt);
  m::data_callback_fn dfn(dummy_data_fn, &data_ctxt);

  u::watchdog watchdog(&efn);

  u::periodic_background_proc<m::model_downloader> bgproc(repeatms, watchdog, "Test model downloader", &efn);

  const auto start = std::chrono::system_clock::now();
  m::model_downloader md(transport.get(), &dfn, nullptr);
  scode = bgproc.init(&md);
  // There needs to be a wait here to ensure stop is not called before the background proc has a chance to run its
  // iteration.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  bgproc.stop();
  const auto stop = std::chrono::system_clock::now();
  const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  BOOST_CHECK_LE(diff.count(), 500);
  BOOST_CHECK_EQUAL(data_ctxt, 20);
}

BOOST_AUTO_TEST_CASE(mock_azure_storage_model_data)
{
  // create a simple ds configuration
  u::configuration cc;
  auto scode = cfg::create_from_json(JSON_CFG, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  cc.set(r::name::EH_TEST, "true");  // local test event hub

  auto http_client = new mock_http_client("http://test.com");
  std::unique_ptr<m::i_data_transport> data_transport(new m::restapi_data_transport(http_client, nullptr));

  r::api_status status;

  m::model_data md;
  BOOST_CHECK_EQUAL(md.refresh_count(), 0);
  scode = data_transport->get_data(md, &status);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  BOOST_CHECK_EQUAL(md.refresh_count(), 1);
  scode = data_transport->get_data(md, &status);
  BOOST_CHECK_EQUAL(md.refresh_count(), 2);
}

BOOST_AUTO_TEST_CASE(mock_azure_storage_model_api_data)
{
  // create a simple ds configuration
  u::configuration cc;
  auto scode = cfg::create_from_json(JSON_CFG, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  cc.set(r::name::EH_TEST, "true");  // local test event hub
  cc.set("model.source", "HTTP_MODEL_DATA");
  cc.set("http.api.key", "apikey1234");

  auto http_client = new mock_http_client("http://test.com");
  std::unique_ptr<m::i_data_transport> data_transport(new m::restapi_data_transport(
      std::unique_ptr<mock_http_client>(http_client), cc, m::model_source::HTTP_API, nullptr));

  r::api_status status;

  m::model_data md;
  BOOST_CHECK_EQUAL(md.refresh_count(), 0);
  scode = data_transport->get_data(md, &status);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  BOOST_CHECK_EQUAL(md.refresh_count(), 1);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  scode = data_transport->get_data(md, &status);
  BOOST_CHECK_EQUAL(md.refresh_count(), 2);
}
#  endif  // USE_AZURE_FACTORIES
#endif    //_WIN32 (http_server http protocol issues in linux)

void register_local_file_factory();
const char* const DUMMY_DATA_TRANSPORT = "DUMMY_DATA_TRANSPORT";
const char* const CFG_PARAM = "model.local.file";

BOOST_AUTO_TEST_CASE(data_transport_user_extention)
{
  register_local_file_factory();
  const u::configuration cc;

  std::unique_ptr<m::i_data_transport> data_transport;
  auto scode = r::data_transport_factory.create(data_transport, DUMMY_DATA_TRANSPORT, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  m::model_data md;
  scode = data_transport->get_data(md);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  md.free();
}

BOOST_AUTO_TEST_CASE(vw_model_factory)
{
  register_local_file_factory();

  u::configuration model_cc;
  model_cc.set(r::name::VW_CMDLINE, "--lda 5");
  std::unique_ptr<m::i_model> vw;
  const auto scode = r::model_factory.create(vw, r::value::VW, model_cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
}

m::model_data get_model_data()
{
  const u::configuration cc;
  std::unique_ptr<m::i_data_transport> pdt;
  r::data_transport_factory.create(pdt, DUMMY_DATA_TRANSPORT, cc);
  m::model_data md;
  const auto scode = pdt->get_data(md);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  return md;
}

class dummy_data_transport : public m::i_data_transport
{
  int get_data(m::model_data& data, r::api_status* status) override
  {
    data.alloc(10);
    return r::error_code::success;
  }
};

int dummy_data_tranport_create(
    std::unique_ptr<m::i_data_transport>& retval, const u::configuration& config, r::i_trace* trace, r::api_status* status)
{
  retval.reset(new dummy_data_transport());
  return r::error_code::success;
}

void register_local_file_factory()
{
  r::data_transport_factory.register_type(DUMMY_DATA_TRANSPORT, dummy_data_tranport_create);
}

BOOST_AUTO_TEST_CASE(vw_model_type)
{
  register_local_file_factory();

  u::configuration model_cc;
  std::unique_ptr<m::i_model> vw;

  model_cc.set(r::name::VW_CMDLINE, "--cb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  BOOST_CHECK_EQUAL(r::error_code::success, r::model_factory.create(vw, r::value::VW, model_cc));
  BOOST_CHECK_EQUAL((int)m::model_type_t::CB, (int)vw->model_type());

  model_cc.set(r::name::VW_CMDLINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  model_cc.set(
      r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  BOOST_CHECK_EQUAL(r::error_code::success, r::model_factory.create(vw, r::value::VW, model_cc));
  BOOST_CHECK_EQUAL((int)m::model_type_t::CCB, (int)vw->model_type());

  model_cc.set(r::name::VW_CMDLINE, "--slates --ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  model_cc.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE,
      "--slates --ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  BOOST_CHECK_EQUAL(r::error_code::success, r::model_factory.create(vw, r::value::VW, model_cc));
  BOOST_CHECK_EQUAL((int)m::model_type_t::SLATES, (int)vw->model_type());
}
