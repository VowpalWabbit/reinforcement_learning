
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include "vw/fb_parser/parse_example_flatbuffer.h"
#include "flatbuffers/flatbuffers.h"
#include "factory_resolver.h"

#include <boost/test/unit_test.hpp>

#include "api_status.h"
#include "rl_string_view.h"
// #include "ca_loop.h"
#include "cb_loop.h"
#include "live_model_impl.h"
// #include "ccb_loop.h"
#include "config_utility.h"
#include "constants.h"
#include "err_constants.h"
#include "live_model.h"
#include "mock_util.h"
#include "model_mgmt.h"
#include "ranking_response.h"
#include "sampling.h"
#include "sender.h"
// #include "slates_loop.h"
#include "str_util.h"

#include "vw/core/vw.h"


#include "prototype_example_root.h"
#include "prototype_label.h"
USE_PROTOTYPE_MNEMONICS

#include <fstream>
#include <thread>
#include <vector>

namespace fb = VW::parsers::flatbuffer;
using namespace flatbuffers;

constexpr float FLOAT_TOL = 0.0001f;
#ifdef __GNUG__

// Fakeit does not work with GCC's devirtualization
// which is enabled with -O2 (the default) or higher.
#  pragma GCC optimize("no-devirtualize")

#endif

#include <fakeit/fakeit.hpp>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace m = reinforcement_learning::model_management;
namespace err = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;

using namespace fakeit;

namespace
{
const auto JSON_CONTEXT = R"({"_multi":[{"Action":{"c":1}},{"Action":{"c":2}}]})";

template <typename MODEL_TYPE>
MODEL_TYPE create_mock_live_model(const u::configuration& config,
    r::data_transport_factory_t* data_transport_factory = nullptr, r::model_factory_t* model_factory = nullptr,
    r::sender_factory_t* sender_factory = nullptr)
{
  static auto mock_sender = get_mock_sender(r::error_code::success);
  static auto mock_data_transport = get_mock_data_transport();
  static auto mock_model = get_mock_model(r::model_management::model_type_t::CB);

  static auto default_sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get());
  static auto default_data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
  static auto default_model_factory = get_mock_model_factory(mock_model.get());

  if (!data_transport_factory) { data_transport_factory = default_data_transport_factory.get(); }

  if (!model_factory) { model_factory = default_model_factory.get(); }

  if (!sender_factory) { sender_factory = default_sender_factory.get(); }

  MODEL_TYPE model(
      config, nullptr, nullptr, &r::trace_logger_factory, data_transport_factory, model_factory, sender_factory);
  return model;
}
}  // namespace

using str_view_t = r::str_view;

str_view_t create_cb_context(VW::workspace& w, FlatBufferBuilder& builder)
{
  using namespace vwtest;

  vwtest::ns shared { "Shared", { { "t_abc", 1. } } };
  vwtest::ns action1 { "Action", { { "c", 2 } } };
  vwtest::ns action2 { "Action", { { "c", 3 } } };

  std::vector<vwtest::ns> shared_ns { shared };
  std::vector<vwtest::ns> action1_ns { action1 };
  std::vector<vwtest::ns> action2_ns { action2 };

  vwtest::example shared_ex { shared_ns, vwtest::cb_label_shared() };
  vwtest::example action1_ex { action1_ns };
  vwtest::example action2_ex { action2_ns };

  vwtest::multiex cb_example
  {{
      shared_ex,
      action1_ex,
      action2_ex
  }};

  Offset<fb::ExampleRoot> root = vwtest::create_example_root(builder, w, cb_example);
  builder.FinishSizePrefixed(root);

  return str_view_t(reinterpret_cast<const char*>(builder.GetBufferPointer()), builder.GetSize());
}


BOOST_AUTO_TEST_CASE(flatbuffer_context_cb_loop_rank_basic)
{
  namespace n = r::name;
  namespace v = r::value;

  // create a simple ds configuration
  u::configuration config;

  config[n::APP_ID] = "TestApp";
  //config[n::MODEL_SRC] = v::NO_MODEL_DATA; (Test mock does the same thing)
  config["IsExplorationEnabled"] = "true";
  config["InitialExplorationEpsilon"] = "1.0";

  // Nothing listens to this, but multiple tests rely on it?!
  //config[n::VW_CMDLINE] = "--cb_explore_adf --epsilon 0.2 --quiet --flatbuffer";
  config[n::MODEL_VW_INITIAL_COMMAND_LINE] = "--cb_explore_adf --epsilon 0.2 --quiet --flatbuffer";

  config[n::EH_TEST] = "true";
  config[n::INPUT_SERIALIZATION] = v::VWFB_INPUT_SERIALIZATION;

  r::api_status status;
  // create the ds live_model, and initialize it with the config
  r::cb_loop ds = create_mock_live_model<r::cb_loop>(config, nullptr, &r::model_factory, nullptr);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  const auto event_id = "event_id";

  auto w = VW::initialize("--cb_explore_adf --epsilon 0.2 --quiet", nullptr, false, nullptr, nullptr);

  FlatBufferBuilder builder;
  auto context = create_cb_context(*w, builder);

  r::ranking_response response;
  // request ranking
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, context, response, &status), err::success);
  BOOST_CHECK_EQUAL(response.size(), 2);

  // valid request => status is reset (check this before the next set of tests, because if this breaks, they will
  // all break)
  r::api_status::try_update(&status, -42, "hello");
  ds.choose_rank(event_id, context, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  // check expected returned codes for basic invalid input
  const auto invalid_event_id = "";
  BOOST_CHECK_EQUAL(
      ds.choose_rank(invalid_event_id, context, response, &status), err::invalid_argument); // invalid event_id

  const auto invalid_context = "";
  BOOST_CHECK_EQUAL(
      ds.choose_rank(event_id, invalid_context, response, &status), err::invalid_argument); // invalid context

  // JSON context is invalid for FB input serialization, but because this happens within the parsing code, rather
  // than argument validation code, we return a general model-side error, rather then the more specific invalid_argument
  // We could in-principle fix this, but it would require a fair bit of changes to the interaction between i_model
  // implementations and RLClientLib, as well as a bit of trickery around sharing the api_status object between
  // RLClientLib and VW.
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, JSON_CONTEXT, response, &status), err::model_rank_error);

  VW::finish(*w);
}

//std::unique_ptr<fakeit::Mock<r::i_sender>> get_mock_sender(std::vector<buffer_data_t>& recorded_messages)
//{
//  auto mock = std::unique_ptr<Mock<r::i_sender>>(new fakeit::Mock<r::i_sender>());
//  const std::function<int(const buffer_t&, r::api_status*&)> send_fn =
//
//      [&recorded_messages](const buffer_t& message, r::api_status*& status)
//  {
//    // take a copy of the data
//    // the 'message' is a shared pointer to an object on the object pool
//    // if live_model dtor is called prior to the 'recorded_messages' vector (as is done in most of the test cases)
//    // there will be an invalid attempt to restore/free the object to/from the destructed object pool
//    recorded_messages.push_back(*message.get());
//    return r::error_code::success;
//  };
//  When(Method((*mock), init)).AlwaysReturn(r::error_code::success);
//  When(Method((*mock), send)).AlwaysDo(send_fn);
//  Fake(Dtor((*mock)));
//
//  return mock;
//}

//BOOST_AUTO_TEST_CASE(flatbuffer_context_cb_loop_rank_logging)
//{
//  namespace n = r::name;
//  namespace v = r::value;
//
//  // create a simple ds configuration
//  u::configuration config;
//
//  config[n::APP_ID] = "TestApp";
//  // config[n::MODEL_SRC] = v::NO_MODEL_DATA; (Test mock does the same thing)
//  config["IsExplorationEnabled"] = "true";
//  config["InitialExplorationEpsilon"] = "1.0";
//
//  // Nothing listens to this, but multiple tests rely on it?!
//  // config[n::VW_CMDLINE] = "--cb_explore_adf --epsilon 0.2 --quiet --flatbuffer";
//  config[n::MODEL_VW_INITIAL_COMMAND_LINE] = "--cb_explore_adf --epsilon 0.2 --quiet --flatbuffer";
//
//  config[n::EH_TEST] = "true";
//  config[n::INPUT_SERIALIZATION] = v::VWFB_INPUT_SERIALIZATION;
//
//  r::api_status status;
//
//
//
//  //r::sender_factory_t sender_factory = get_mock_sender_factory()
//
//  // create the ds live_model, and initialize it with the config
//  r::cb_loop ds = create_mock_live_model<r::cb_loop>(config, nullptr, &r::model_factory, nullptr);
//  BOOST_CHECK_EQUAL(ds.init(&status), err::success);
//
//  const auto event_id = "event_id";
//
//  auto w = VW::initialize("--cb_explore_adf --epsilon 0.2 --quiet", nullptr, false, nullptr, nullptr);
//
//  FlatBufferBuilder builder;
//  auto context = create_cb_context(*w, builder);
//
//  r::ranking_response response;
//  // request ranking
//  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, context, response, &status), err::success);
//  BOOST_CHECK_EQUAL(response.size(), 2);
//
//
//}

#pragma region fb_context_parse_test



#pragma endregion // fb_context_parse_test

