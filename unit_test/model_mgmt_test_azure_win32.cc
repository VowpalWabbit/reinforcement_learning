#include "model_mgmt_test.h"

int get_export_frequency(const u::configuration& cc, int& interval_ms, r::api_status* status) {
  const auto export_freq_s = cc.get("ModelExportFrequency", nullptr);
  if ( export_freq_s == nullptr ) {
    RETURN_ERROR_LS(nullptr, status, model_export_frequency_not_provided);
  }
  // hh:mm:ss
  const std::regex re("\\s*([0-9]+):([0-9]+):([0-9]+)\\s*");
  std::cmatch m;
  if(std::regex_match(export_freq_s,m,re)) {
    const auto hrs  = atoi(m[1].str().c_str());
    const auto mins = atoi(m[2].str().c_str());
    const auto secs = atoi(m[3].str().c_str());
    interval_ms = hrs * 60 * 60 * 1000 + mins * 60 * 1000 + secs * 1000;
    if ( interval_ms == 0 ) {
      RETURN_ERROR_LS(nullptr, status, bad_time_interval);
    }
    return e::success;
  }
  else {
    RETURN_ERROR_LS(nullptr, status, bad_time_interval);
  }
}

void dummy_error_fn(const r::api_status& err, void* ctxt) {
  *( (int*)ctxt ) = 10;
}

void dummy_data_fn(const m::model_data& data, int* ctxt) {
  *ctxt = 20;
}

#ifdef _WIN32 //_WIN32 (http_server http protocol issues in linux)
#ifdef USE_AZURE_FACTORIES
BOOST_AUTO_TEST_CASE(background_mock_azure_get) {
  //create a simple ds configuration
  u::configuration cc;
  auto scode = cfg::create_from_json(JSON_CFG,cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  cc.set(r::name::EH_TEST, "true"); // local test event hub
  cc.set("ModelExportFrequency", "00:01:00");

  auto http_client = new mock_http_client("http://test.com");
  std::unique_ptr<m::i_data_transport> transport(new m::restapi_data_transport(http_client, nullptr));

  r::api_status status;

  int repeatms;
  get_export_frequency(cc, repeatms, &status);

  int err_ctxt;
  int data_ctxt;
  r::error_callback_fn efn(dummy_error_fn,&err_ctxt);
  m::data_callback_fn dfn(dummy_data_fn, &data_ctxt);

  u::watchdog watchdog(&efn);

  u::periodic_background_proc<m::model_downloader> bgproc(repeatms, watchdog, "Test model downloader", &efn);

  const auto start = std::chrono::system_clock::now();
  m::model_downloader md(transport.get(), &dfn,nullptr);
  scode = bgproc.init(&md);
  // There needs to be a wait here to ensure stop is not called before the background proc has a chance to run its iteration.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  bgproc.stop();
  const auto stop = std::chrono::system_clock::now();
  const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>( stop - start );
  BOOST_CHECK_LE(diff.count(), 500);
  BOOST_CHECK_EQUAL(data_ctxt, 20);
}

BOOST_AUTO_TEST_CASE(mock_azure_storage_model_data)
{
  //create a simple ds configuration
  u::configuration cc;
  auto scode = cfg::create_from_json(JSON_CFG,cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  cc.set(r::name::EH_TEST, "true"); // local test event hub

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
#endif // USE_AZURE_FACTORIES
#endif //_WIN32 (http_server http protocol issues in linux)