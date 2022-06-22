#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include "logger/file/file_logger.h"
#include <boost/test/unit_test.hpp>

#include "err_constants.h"

#include <cstdio>

namespace rl = reinforcement_learning;
namespace rlog = reinforcement_learning::logger;
namespace rerr = reinforcement_learning::error_code;
namespace rutil = reinforcement_learning::utility;

bool file_exists(const std::string& file)
{
  std::ifstream f(file);
  return f.good();
}

BOOST_AUTO_TEST_CASE(file_logger_test)
{
  const std::string file("file_logger_test");
  {
    if (file_exists(file)) remove(file.c_str());

    BOOST_CHECK(!file_exists(file));

    rlog::file::file_logger logger(file, nullptr);
    rutil::configuration config;
    BOOST_CHECK_EQUAL(logger.init(config, nullptr), rerr::success);
    const auto buff = rl::i_sender::buffer(new rutil::data_buffer());
    logger.send(buff);
  }

  BOOST_CHECK(file_exists(file));
  remove(file.c_str());
}