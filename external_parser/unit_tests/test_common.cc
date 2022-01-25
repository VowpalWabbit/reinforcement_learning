#include "test_common.h"

#include "io/io_adapter.h"
#include "parser.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace endian {
bool is_big_endian(void) {
  const union {
    std::uint32_t i;
    char c[4];
  } b_int{0x01000000};
  return b_int.c[0] == 1;
}

uint32_t from_network_order(std::uint32_t net_l) {
  if (is_big_endian()) {
    return net_l;
  }

  uint32_t ret_val;
  uint8_t *p_ret_raw = (uint8_t *)&ret_val;
  uint8_t *p_raw = (uint8_t *)&net_l;
  p_ret_raw[0] = p_raw[3];
  p_ret_raw[1] = p_raw[2];
  p_ret_raw[2] = p_raw[1];
  p_ret_raw[3] = p_raw[0];
  return ret_val;
}
} // end of namespace endian

void set_slates_label(v_array<example *> &examples) {
  examples[0]->pred.decision_scores.resize(2);
  examples[0]->pred.decision_scores[0].push_back({0, 0.f});
  examples[0]->pred.decision_scores[1].push_back({1, 0.f});
}

void clear_examples(v_array<example *> &examples, vw *vw) {
  if (vw->l->is_multiline()) {
    multi_ex multi_exs;
    for (auto *ex : examples) {
      multi_exs.push_back(ex);
    }
    vw->finish_example(multi_exs);
    multi_exs.clear();
  } else {
    for (auto *ex : examples) {
      VW::finish_example(*vw, *ex);
    }
  }
  examples.clear();
}

void set_buffer_as_vw_input(const std::vector<char> &buffer, vw *vw) {
  vw->example_parser->input.close_files();
  vw->example_parser->input.add_file(
      VW::io::create_buffer_view(buffer.data(), buffer.size()));
}

std::vector<char> read_file(std::string file_name) {
  std::ifstream file;
  file.open(file_name, std::ios::binary | std::ios::ate);
  BOOST_REQUIRE_EQUAL(file.is_open(), true);
  std::streamsize ssize = file.tellg();
  file.seekg(0);

  // if we can't read the file then fail
  BOOST_REQUIRE_GT(ssize, 0);

  std::vector<char> buffer(ssize);
  file.read(buffer.data(), ssize);
  file.close();
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

uint32_t get_payload_size(std::vector<char> &buffer) {
  std::vector<char> size_buffer = {buffer.begin() + 4, buffer.begin() + 8};
  return endian::from_network_order(
      *reinterpret_cast<const uint32_t *>(size_buffer.data()));
}

std::vector<const v2::JoinedEvent *> wrap_into_joined_events(
    std::vector<char> &buffer,
    std::vector<flatbuffers::DetachedBuffer> &detached_buffers) {
  const int PREAMBLE_LENGTH = 8;
  // if file is smaller than preamble size then fail
  BOOST_REQUIRE_GT(buffer.size(), PREAMBLE_LENGTH);

  flatbuffers::FlatBufferBuilder fbb;
  std::vector<const v2::JoinedEvent *> event_list{};
  size_t event_index = 0;

  while (buffer.size() > PREAMBLE_LENGTH) {
    uint32_t payload_size = get_payload_size(buffer);
    // throw away first 8 bytes as they contain preamble (todo maybe verify
    // preamble)
    std::vector<char> event_batch_buffer = {buffer.begin() + PREAMBLE_LENGTH,
                                            buffer.begin() + PREAMBLE_LENGTH +
                                                payload_size};
    auto event_batch = v2::GetEventBatch(event_batch_buffer.data());
    BOOST_REQUIRE_GE(event_batch->events()->size(), 1);

    int day = 30;
    v2::TimeStamp ts(2020, 3, day, 10, 20, 30, 0);

    for (size_t i = 0; i < event_batch->events()->size(); i++) {
      const auto *event_payload = event_batch->events()->Get(i)->payload();
      auto vec = fbb.CreateVector(event_payload->data(), event_payload->size());

      if (i < 30) {
        ts.mutate_day(day - i);
      }

      auto fb = v2::CreateJoinedEvent(fbb, vec, &ts);
      fbb.Finish(fb);
      detached_buffers.push_back(fbb.Release());

      const v2::JoinedEvent *je = flatbuffers::GetRoot<v2::JoinedEvent>(
          detached_buffers[event_index].data());

      event_list.push_back(je);
      event_index++;
    }

    buffer = {buffer.begin() + PREAMBLE_LENGTH + payload_size, buffer.end()};
  }

  return event_list;
}
