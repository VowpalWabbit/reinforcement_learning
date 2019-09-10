#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Main
#include <boost/test/unit_test.hpp>

#include "onnx_extension.h"

struct GlobalConfig {
  GlobalConfig()
  {
    reinforcement_learning::onnx::register_onnx_factory();
  }

  ~GlobalConfig()
  {
  }
};

BOOST_GLOBAL_FIXTURE(GlobalConfig);
