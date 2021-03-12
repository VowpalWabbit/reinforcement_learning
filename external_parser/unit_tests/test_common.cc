#include "test_common.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

void clear_examples(v_array<example *> &examples, vw *vw) {
  for (auto *ex : examples) {
    VW::finish_example(*vw, *ex);
  }
  examples.clear();
}

std::vector<char> read_file(std::string file_name) {
  std::ifstream file(file_name, std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0);

  BOOST_REQUIRE_GT(size, 0);

  std::vector<char> buffer(size);
  // if we can't read the file then fail
  file.read(buffer.data(), size);
  return buffer;
}

std::string get_test_files_location() {
  if (boost::unit_test::framework::master_test_suite().argc <= 1) {
    // find default value of input files
    // TODO this might not work when windows build is added
    boost::filesystem::path p(__FILE__);
    p.remove_filename();
    return p.string() + "/test_files/";
  } else {
    return boost::unit_test::framework::master_test_suite().argv[1];
  }
}

const v2::JoinedEvent *
wrap_into_joined_event(std::vector<char> &buffer,
                       flatbuffers::DetachedBuffer &detached_buffer) {
  flatbuffers::FlatBufferBuilder fbb;

  // throw away first 8 bytes as they contain preamble (todo maybe verify
  // preamble)
  BOOST_REQUIRE_GT(buffer.size(), 8);

  std::vector<char> non_preamble_buffer = {buffer.begin() + 8, buffer.end()};
  auto event_batch = v2::GetEventBatch(non_preamble_buffer.data());
  // assuming one event in eventbatch, TODO extend
  BOOST_REQUIRE_GE(event_batch->events()->size(), 1);

  const auto *payload = event_batch->events()->Get(0)->payload();
  auto vec = fbb.CreateVector(payload->data(), payload->size());
  auto fb = v2::CreateJoinedEvent(fbb, vec, nullptr);
  fbb.Finish(fb);
  detached_buffer = fbb.Release();
  return flatbuffers::GetRoot<v2::JoinedEvent>(detached_buffer.data());
}