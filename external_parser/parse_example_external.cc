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

namespace VW {
external_parser::external_parser(vw *all)
    : _example_joiner(all, non_default_reward_calc) {
  std::cout << "hey" << std::endl;
}

bool external_parser::parse_examples(vw *all, v_array<example *> &examples) {

  char *line = nullptr;
  unsigned int payload_type;
  size_t buffer_length = 4 * sizeof(char);

  if (!_header_read) {
    // TODO change this to handle multiple files if needed
    const std::vector<char> magic = {'V', 'W', 'F', 'B'};
    // read the 4 magic bytes
    auto len = all->example_parser->input->buf_read(line, buffer_length);

    if (len < buffer_length) {
      return false;
    }

    std::vector<char> buffer = {line, line + buffer_length};

    if (buffer != magic) {
      return false;
    }

    // read the version
    len = all->example_parser->input->buf_read(line, buffer_length);

    if (len < buffer_length) {
      return false;
    }

    if (*line != 1) {
      return false;
    }

    // payload type, check for header
    len = all->example_parser->input->buf_read(line, sizeof(payload_type));

    if (len < sizeof(payload_type)) {
      return false;
    }

    payload_type = *reinterpret_cast<const unsigned int *>(line);
    if (payload_type != MSG_TYPE_HEADER) {
      return false;
    }

    // read header size
    len = all->example_parser->input->buf_read(line, buffer_length);
    if (len < buffer_length) {
      return false;
    }

    uint32_t payload_size = payload_type =
        *reinterpret_cast<const uint32_t *>(line);
    ;
    // read the payload
    len = all->example_parser->input->buf_read(line, payload_size);

    if (len < payload_size) {
      return false;
    }

    std::vector<char> payload = {line, line + payload_size};
    // TODO:: deserialize header read_header(payload);
    _header_read = true;
  }

  if (_example_joiner.processing_batch()) {
    _example_joiner.train_on_joined(examples);
    return true;
  }

  auto len = all->example_parser->input->buf_read(line, sizeof(payload_type));

  if (len < sizeof(payload_type)) {
    return false;
  }

  payload_type = *reinterpret_cast<const unsigned int *>(line);
  if (payload_type == 0) {
    // try again?
    auto len = all->example_parser->input->buf_read(line, sizeof(payload_type));

    if (len < sizeof(payload_type)) {
      return false;
    }

    payload_type = *reinterpret_cast<const unsigned int *>(line);
  }

  if (payload_type != MSG_TYPE_EOF) {
    if (payload_type != MSG_TYPE_REGULAR)
      return false;
    // read payload size
    len = all->example_parser->input->buf_read(line, sizeof(uint32_t));
    if (len < buffer_length) {
      return false;
    }

    uint32_t payload_size = *reinterpret_cast<const uint32_t *>(line);
    // read the payload
    len = all->example_parser->input->buf_read(line, payload_size);

    if (len < payload_size) {
      return false;
    }

    _payload = {line, line + payload_size};

    auto joined_payload =
        flatbuffers::GetRoot<v2::JoinedPayload>(_payload.data());
    for (size_t i = 0; i < joined_payload->events()->size(); i++) {
      // process events one-by-one
      _example_joiner.process_event(*joined_payload->events()->Get(i));
    }
    _example_joiner.train_on_joined(examples);
    return true;
  }
  return false;
}

int parse_examples(vw *all, v_array<example *> &examples) {
  return static_cast<int>(all->external_parser->parse_examples(all, examples));
}

} // namespace VW
