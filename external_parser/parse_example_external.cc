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
#include "parse_example_external.h"

// we could add any joiner logic that we want and have a flag or parameter that
// desides which method to select
int non_default_reward_calc(const joined_event &event,
                            v_array<example *> &examples) {
  int index = event.interaction_data.actions[0];
  examples[index]->l.cb.costs.push_back(
      {1.0f, event.interaction_data.actions[index - 1],
       event.interaction_data.probabilities[index - 1]});
  std::cout << "this is a different reward logic that does some dummy reward "
               "calculation"
            << std::endl;

  return 0;
}

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

// helpers end

namespace VW {
external_parser::external_parser(vw *all)
    : _example_joiner(all, non_default_reward_calc) {}

bool external_parser::parse_examples(vw *all, v_array<example *> &examples) {
  unsigned int payload_type;
  uint32_t payload_size;
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
    if (!read_payload_size(all->example_parser->input, payload_size)) {
      return false;
    }

    // read the payload
    if (!read_payload(all->example_parser->input, payload, payload_size)) {
      return false;
    }

    // std::vector<char> payload = {payload, payload + payload_size};
    // TODO:: deserialize header read_header(payload);
    _header_read = true;
  }

  // either process next id from an ongoing batch
  // or read the next batch from file
  if (_example_joiner.processing_batch()) {
    _example_joiner.process_joined(examples);
    return true;
  }

  if (!read_payload_type(all->example_parser->input, payload_type)) {
    return false;
  }

  if (payload_type == 0) {
    // try again?
    if (!read_payload_type(all->example_parser->input, payload_type)) {
      return false;
    }
  }

  if (payload_type != MSG_TYPE_EOF) {
    if (payload_type != MSG_TYPE_REGULAR)
      return false;
    // read payload size
    if (!read_payload_size(all->example_parser->input, payload_size)) {
      return false;
    }
    // read the payload
    if (!read_payload(all->example_parser->input, payload, payload_size)) {
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

int parse_examples(vw *all, v_array<example *> &examples) {
  return static_cast<int>(all->external_parser->parse_examples(all, examples));
}

} // namespace VW
