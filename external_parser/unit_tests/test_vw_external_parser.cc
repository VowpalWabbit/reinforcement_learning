#define BOOST_TEST_DYN_LINK

#include "io/io_adapter.h"
#include "parser.h"
#include "test_common.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(cb_simple) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/cb_simple.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  v_array<example *> examples;

  io_buf *reader_view_of_buffer = new io_buf();
  delete vw->example_parser->input;
  vw->example_parser->input = reader_view_of_buffer;

  examples.push_back(&VW::get_unused_example(vw));
  reader_view_of_buffer->add_file(
      VW::io::create_buffer_view(buffer.data(), buffer.size()));

  while (vw->example_parser->reader(vw, examples) > 0) {
    // TODO examine example internals here, this is what vw will get before
    // calling learn
    BOOST_CHECK_EQUAL(examples.size(), 4);

    // simulate next call to parser->read by clearing up examples
    // and preparing one unused example
    clear_examples(examples, vw);
    examples.push_back(&VW::get_unused_example(vw));
  }

  clear_examples(examples, vw);
  VW::finish(*vw);
}