#include "test_common.h"

#include "io/io_adapter.h"
#include "parser.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

void clear_examples(v_array<example *> &examples, vw *vw) {
  for (auto *ex : examples) {
    VW::finish_example(*vw, *ex);
  }
  examples.clear();
}

void set_buffer_as_vw_input(const std::vector<char> &buffer, vw *vw) {
  io_buf *reader_view_of_buffer = new io_buf();
  delete vw->example_parser->input;
  vw->example_parser->input = reader_view_of_buffer;

  reader_view_of_buffer->add_file(
    VW::io::create_buffer_view(buffer.data(), buffer.size()));
}

std::vector<char> read_file(std::string file_name) {
  std::ifstream file(file_name, std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0);

  // if we can't read the file then fail
  BOOST_REQUIRE_GT(size, 0);

  std::vector<char> buffer(size);
  file.read(buffer.data(), size);
  return buffer;
}

std::string get_test_files_location() {
  if (boost::unit_test::framework::master_test_suite().argc <= 1) {
    // set default location of test_files dir
    // TODO this might not work when windows build is added
    boost::filesystem::path p(__FILE__);
    p.remove_filename();
    return p.string() + "/test_files/";
  } else {
    // test_files dir set from command line
    return boost::unit_test::framework::master_test_suite().argv[1];
  }
}

const v2::JoinedEvent *
wrap_into_joined_event(std::vector<char> &buffer,
                       flatbuffers::DetachedBuffer &detached_buffer) {
  flatbuffers::FlatBufferBuilder fbb;

  // if file is smaller than preamble size then fail
  BOOST_REQUIRE_GT(buffer.size(), 8);

  // throw away first 8 bytes as they contain preamble (todo maybe verify
  // preamble)
  std::vector<char> non_preamble_buffer = {buffer.begin() + 8, buffer.end()};
  auto event_batch = v2::GetEventBatch(non_preamble_buffer.data());

  BOOST_REQUIRE_GE(event_batch->events()->size(), 1);

  // TODO read all the event's from batch and not just the first one
  const auto *payload = event_batch->events()->Get(0)->payload();
  auto vec = fbb.CreateVector(payload->data(), payload->size());

  v2::TimeStamp ts(2020, 3, 18, 10, 20, 30, 0);
  auto fb = v2::CreateJoinedEvent(fbb, vec, &ts);
  fbb.Finish(fb);
  detached_buffer = fbb.Release();
  return flatbuffers::GetRoot<v2::JoinedEvent>(detached_buffer.data());
}