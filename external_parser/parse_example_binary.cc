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
#include "io/logger.h"
#include "memory.h"
#include "parse_example_binary.h"

// TODO need to check if errors will be detected from stderr/stdout/other and
// use appropriate logger

// helpers start
bool read_payload_type(io_buf *input, unsigned int &payload_type) {
  char *line = nullptr;
  auto len = input->buf_read(line, sizeof(unsigned int));

  if (len < sizeof(unsigned int) || line == nullptr) {
    if (len == 0) {
      // when we are trying to fetch the next payload and we find out that there
      // is nothing left to read the file doesn't have to necessarily contain an
      // EOF
      VW::io::logger::log_info("Reached end of file");
      payload_type = MSG_TYPE_EOF;
      return true;
    }
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

bool read_payload(io_buf *input, char *&payload, uint32_t payload_size) {
  char *line = nullptr;
  auto len = input->buf_read(line, payload_size);

  if (len < payload_size || line == nullptr) {
    return false;
  }
  payload = line;
  return true;
}

bool read_padding(io_buf *input, uint32_t previous_payload_size,
                  uint32_t &padding_bytes) {
  char *line = nullptr;
  padding_bytes = previous_payload_size % 8;
  if (padding_bytes > 0) {
    // read and discard padding bytes
    return read_payload(input, line, padding_bytes);
  }
  return true;
}

// helpers end

namespace VW {
namespace external {
binary_parser::binary_parser(vw *all)
    : _header_read(false), _example_joiner(all), _payload(nullptr),
      _payload_size(0), _total_size_read(0) {}

binary_parser::binary_parser(vw *all, bool binary_to_json, std::string outfile_name)
    : _header_read(false), _example_joiner(all, binary_to_json, outfile_name),
      _payload(nullptr), _payload_size(0), _total_size_read(0) {}

binary_parser::~binary_parser(){}

bool binary_parser::read_magic(io_buf *input) {
  const uint32_t buffer_length = 4 * sizeof(char);
  // read the 4 magic bytes
  if (!read_payload(input, _payload, buffer_length)) {
    VW::io::logger::log_critical(
        "Failed to read payload while reading magic, after having read [{}] "
        "bytes from the file",
        _total_size_read);
    return false;
  }

  _total_size_read += buffer_length;

  std::vector<char> buffer = {_payload, _payload + buffer_length};
  const std::vector<char> magic = {'V', 'W', 'F', 'B'};
  if (buffer != magic) {
    VW::io::logger::log_critical("Magic bytes in file are incorrect");
    return false;
  }
  return true;
}

bool binary_parser::read_version(io_buf *input) {
  _payload = nullptr;
  const uint32_t buffer_length = 4 * sizeof(char);
  if (!read_payload(input, _payload, buffer_length)) {
    VW::io::logger::log_critical("Failed to read payload while reading file "
                                 "version, after having read [{}] "
                                 "bytes from the file",
                                 _total_size_read);
    return false;
  }

  _total_size_read += buffer_length;

  if (*_payload != BINARY_PARSER_VERSION) {
    VW::io::logger::log_critical(
        "File version [{}] does not match the parser version [{}]",
        static_cast<size_t>(*_payload), BINARY_PARSER_VERSION);
    return false;
  }
  return true;
}

bool binary_parser::read_header(io_buf *input) {
  unsigned int payload_type;
  _payload = nullptr;
  if (!read_payload_type(input, payload_type)) {
    VW::io::logger::log_critical(
        "Failed to read payload while reading header message"
        ", after having read [{}] "
        "bytes from the file",
        _total_size_read);
    return false;
  }

  _total_size_read += sizeof(payload_type);

  if (payload_type != MSG_TYPE_HEADER) {
    VW::io::logger::log_critical("MSG_TYPE_HEADER missing from file");
    return false;
  }

  // read header size
  if (!read_payload_size(input, _payload_size)) {
    VW::io::logger::log_critical(
        "Failed to read header message payload size, after having read "
        "[{}] bytes from the file",
        _total_size_read);
    return false;
  }

  _total_size_read += sizeof(_payload_size);

  // read the payload
  if (!read_payload(input, _payload, _payload_size)) {
    VW::io::logger::log_critical(
        "Failed to read header message payload of size [{}], after having read "
        "[{}] bytes from the file",
        _payload_size, _total_size_read);
    return false;
  }

  _total_size_read += _payload_size;

  // TODO:: consume header

  return true;
}

bool binary_parser::read_checkpoint_msg(io_buf *input) {
  _payload = nullptr;
  if (!read_payload_size(input, _payload_size)) {
    VW::io::logger::log_critical(
        "Failed to read checkpoint message payload size, after having read "
        "[{}] bytes from the file",
        _total_size_read);
    return false;
  }

  _total_size_read += sizeof(_payload_size);

  if (!read_payload(input, _payload, _payload_size)) {
    VW::io::logger::log_critical(
        "Failed to read reward message payload of size [{}], after having read "
        "[{}] bytes from the file",
        _payload_size, _total_size_read);
    return false;
  }

  _total_size_read += _payload_size;

  // TODO: fb verification: what if verification fails, crash or default to
  // something sensible?
  auto checkpoint_info = flatbuffers::GetRoot<v2::CheckpointInfo>(_payload);
  _example_joiner.set_reward_function(checkpoint_info->reward_function_type());
  _example_joiner.set_default_reward(checkpoint_info->default_reward());
  _example_joiner.set_learning_mode_config(checkpoint_info->learning_mode_config());
  _example_joiner.set_problem_type_config(checkpoint_info->problem_type_config());

  return true;
}

bool binary_parser::read_regular_msg(io_buf *input,
                                     v_array<example *> &examples) {
  _payload = nullptr;
  if (!read_payload_size(input, _payload_size)) {
    VW::io::logger::log_critical(
        "Failed to read regular message payload size, after having read "
        "[{}] bytes from the file",
        _total_size_read);
    return false;
  }

  _total_size_read += sizeof(_payload_size);

  if (!read_payload(input, _payload, _payload_size)) {
    VW::io::logger::log_critical("Failed to read regular message payload of "
                                 "size [{}], after having read "
                                 "[{}] bytes from the file",
                                 _payload_size, _total_size_read);
    return false;
  }

  _total_size_read += _payload_size;

  auto joined_payload = flatbuffers::GetRoot<v2::JoinedPayload>(_payload);
  auto verifier =
      flatbuffers::Verifier(reinterpret_cast<const uint8_t *>(_payload),
                            static_cast<size_t>(_payload_size));
  if (!joined_payload->Verify(verifier)) {
    VW::io::logger::log_error(
        "JoinedPayload of size [{}] verification failed after having read [{}] "
        "bytes from the file, skipping JoinedPayload",
        _payload_size, _total_size_read);
    return false;
  }

  for (size_t i = 0; i < joined_payload->events()->size(); i++) {
    // process and group events in batch
    _example_joiner.process_event(*joined_payload->events()->Get(i));
  }
  _example_joiner.process_joined(examples);

  return true;
}

bool binary_parser::advance_to_next_payload_type(io_buf *input,
                                                 unsigned int &payload_type) {
  // read potential excess padding after last payload read
  uint32_t padding;
  if (!read_padding(input, _payload_size, padding)) {
    VW::io::logger::log_critical(
        "Failed to read padding of size [{}], after having read "
        "[{}] bytes from the file",
        padding, _total_size_read);
    return false;
  }

  _total_size_read += padding;

  if (!read_payload_type(input, payload_type)) {
    VW::io::logger::log_warn(
        "Failed to read next payload type from file, after having read "
        "[{}] bytes from the file",
        _total_size_read);
    return false;
  }
  _total_size_read += sizeof(payload_type);
  return true;
}

bool binary_parser::parse_examples(vw *all, v_array<example *> &examples) {
  if (!_header_read) {
    // TODO change this to handle multiple files if needed?
    if (!read_magic(all->example_parser->input.get())) {
      return false;
    }

    if (!read_version(all->example_parser->input.get())) {
      return false;
    }

    if (!read_header(all->example_parser->input.get())) {
      return false;
    }

    _header_read = true;
  }

  // either process next id from an ongoing batch
  // or read the next batch from file
  if (_example_joiner.processing_batch()) {
    _example_joiner.process_joined(examples);
    return true;
  }

  unsigned int payload_type;

  if (!advance_to_next_payload_type(all->example_parser->input.get(), payload_type)) {
    return false;
  }

  if (payload_type == MSG_TYPE_CHECKPOINT) {
    if (!read_checkpoint_msg(all->example_parser->input.get())) {
      return false;
    }

    if (!advance_to_next_payload_type(all->example_parser->input.get(),
                                      payload_type)) {
      return false;
    }
  }

  while (payload_type == MSG_TYPE_REGULAR) {
    if (read_regular_msg(all->example_parser->input.get(), examples)) {
      return true;
    }
    // bad payload process the next one
    if (!advance_to_next_payload_type(all->example_parser->input.get(),
                                      payload_type)) {
      return false;
    }
  }
  if (payload_type != MSG_TYPE_EOF) {
    VW::io::logger::log_critical(
        "Payload type not recognized [{}], after having read [{}] "
        "bytes from the file",
        payload_type, _total_size_read);
  }
  return false;
}
} // namespace external
} // namespace VW
