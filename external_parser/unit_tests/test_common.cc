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
  auto reader_view_of_buffer = VW::make_unique<io_buf>();
  vw->example_parser->input.reset();
  vw->example_parser->input = std::move(reader_view_of_buffer);
  vw->example_parser->input->add_file(
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
    boost::filesystem::path p(__FILE__);
    p.remove_filename();
    return p.string() + "/test_files/";
  } else {
    // test_files dir set from command line
    return boost::unit_test::framework::master_test_suite().argv[1];
  }
}

std::vector<const v2::JoinedEvent *>
wrap_into_joined_events(std::vector<char> &buffer,
                        std::vector<flatbuffers::DetachedBuffer> &detached_buffers) {
  flatbuffers::FlatBufferBuilder fbb;

  // if file is smaller than preamble size then fail
  BOOST_REQUIRE_GT(buffer.size(), 8);

  // throw away first 8 bytes as they contain preamble (todo maybe verify
  // preamble)
  std::vector<char> non_preamble_buffer = {buffer.begin() + 8, buffer.end()};
  auto event_batch = v2::GetEventBatch(non_preamble_buffer.data());

  BOOST_REQUIRE_GE(event_batch->events()->size(), 1);

  std::vector<const v2::JoinedEvent *> event_list {};

  int day = 30;
  v2::TimeStamp ts(2020, 3, day, 10, 20, 30, 0);

  for (size_t i = 0; i < event_batch->events()->size(); i++) {
    const auto *payload = event_batch->events()->Get(i)->payload();
    auto vec = fbb.CreateVector(payload->data(), payload->size());

    if (i < 30) {
      ts.mutate_day(day - i);
    }

    auto fb = v2::CreateJoinedEvent(fbb, vec, &ts);

    fbb.Finish(fb);
    detached_buffers.push_back(fbb.Release());
    const v2::JoinedEvent *je = flatbuffers::GetRoot<v2::JoinedEvent>(detached_buffers[i].data());
    event_list.push_back(je);
  }

  return event_list;
}