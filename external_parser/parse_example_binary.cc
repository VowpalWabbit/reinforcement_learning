// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cfloat>
#include <fstream>
#include <iostream>

#include "action_score.h"
#include "best_constant.h"
#include "cb.h"
#include "constant.h"
#include "flatbuffers/flatbuffers.h"
#include "global_data.h"
#include "memory.h"
#include "parse_example_binary.h"

// helpers start
bool read_payload_type(io_buf *input, unsigned int &payload_type) {
  char *line = nullptr;
  auto len = input->buf_read(line, sizeof(unsigned int));

  if (len < sizeof(unsigned int) || line == nullptr) {
    return false;
  }

  payload_type = *reinterpret_cast<const unsigned int *>(line);
  return true;
}

bool read_payload_size(io_buf *input, uint32_t &payload_size) {
  char *line = nullptr;
  auto len = input->buf_read(line, sizeof(uint32_t));
  if (len < sizeof(uint32_t) || line == nullptr) {
    return false;
  }

  payload_size = *reinterpret_cast<const uint32_t *>(line);
  return true;
}

bool read_payload(io_buf *input, char *&payload, size_t payload_size) {
  char *line = nullptr;
  auto len = input->buf_read(line, payload_size);

  if (len < payload_size || line == nullptr) {
    return false;
  }
  payload = line;
  return true;
}

bool read_padding(io_buf *input, size_t previous_payload_size) {
  char *line = nullptr;
  auto padding_bytes = previous_payload_size % 8;
  if (padding_bytes > 0) {
    // read and discard padding bytes
    return read_payload(input, line, padding_bytes);
  }
  return true;
}

// helpers end

namespace VW {
namespace external {
binary_parser::binary_parser(vw *all) : _example_joiner(all) {}

binary_parser::~binary_parser(){};

bool binary_parser::parse_examples(vw *all, v_array<example *> &examples) {
  unsigned int payload_type;
  char *payload = nullptr;
  const size_t buffer_length = 4 * sizeof(char);

  if (!_header_read) {
    // TODO change this to handle multiple files if needed?
    const std::vector<char> magic = {'V', 'W', 'F', 'B'};
    // read the 4 magic bytes
    if (!read_payload(all->example_parser->input, payload, buffer_length)) {
      return false;
    }

    std::vector<char> buffer = {payload, payload + buffer_length};

    if (buffer != magic) {
      return false;
    }

    // read the version
    if (!read_payload(all->example_parser->input, payload, buffer_length)) {
      return false;
    }

    if (*payload != 1) {
      return false;
    }

    // payload type, check for header
    if (!read_payload_type(all->example_parser->input, payload_type)) {
      return false;
    }

    if (payload_type != MSG_TYPE_HEADER) {
      return false;
    }

    // read header size
    if (!read_payload_size(all->example_parser->input, _payload_size)) {
      return false;
    }

    // read the payload
    if (!read_payload(all->example_parser->input, payload, _payload_size)) {
      return false;
    }

    // std::vector<char> payload = {payload, payload + _payload_size};
    // TODO:: deserialize header read_header(payload);
    _header_read = true;
  }

  // either process next id from an ongoing batch
  // or read the next batch from file
  if (_example_joiner.processing_batch()) {
    _example_joiner.process_joined(examples);
    return true;
  }

  // read potential excess padding after last payload read
  if (!read_padding(all->example_parser->input, _payload_size)) {
    return false;
  }

  if (!read_payload_type(all->example_parser->input, payload_type)) {
    return false;
  }

  if (payload_type == MSG_TYPE_REWARD_FUNCTION) {
    // read payload size
    if (!read_payload_size(all->example_parser->input, _payload_size)) {
      return false;
    }
    // read the payload
    if (!read_payload(all->example_parser->input, payload, _payload_size)) {
      return false;
    }

    auto reward_function_info =
        flatbuffers::GetRoot<v2::RewardFunctionInfo>(payload);
    v2::RewardFunctionType reward_function_type = reward_function_info->type();
    float default_reward = reward_function_info->default_reward();

    std::cout << "reward function type: " << reward_function_type << std::endl;
    std::cout << "default reward: " << default_reward << std::endl;

    _example_joiner.set_reward_function(reward_function_type);
    _example_joiner.set_default_reward(default_reward);
  }

  // read potential excess padding after last payload read
  if (!read_padding(all->example_parser->input, _payload_size)) {
    return false;
  }

  if (!read_payload_type(all->example_parser->input, payload_type)) {
    return false;
  }

  if (payload_type != MSG_TYPE_EOF) {
    if (payload_type != MSG_TYPE_REGULAR)
      return false;
    // read payload size
    if (!read_payload_size(all->example_parser->input, _payload_size)) {
      return false;
    }
    // read the payload
    if (!read_payload(all->example_parser->input, payload, _payload_size)) {
      return false;
    }

    auto joined_payload = flatbuffers::GetRoot<v2::JoinedPayload>(payload);
    for (size_t i = 0; i < joined_payload->events()->size(); i++) {
      // process and group events in batch
      _example_joiner.process_event(*joined_payload->events()->Get(i));
    }
    _example_joiner.process_joined(examples);

    return true;
  }
  return false;
}

} // namespace external
} // namespace VW
