// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "parse_example_binary.h"

#include "flatbuffers/flatbuffers.h"
#include "joiners/example_joiner.h"
#include "vw/core/action_score.h"
#include "vw/core/best_constant.h"
#include "vw/core/cb.h"
#include "vw/core/constant.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/memory.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <fstream>
#include <iostream>

// TODO need to check if errors will be detected from stderr/stdout/other and
// use appropriate logger

// helpers start
bool read_payload_type(io_buf& input, unsigned int& payload_type)
{
  char* line = nullptr;
  auto len = input.buf_read(line, sizeof(unsigned int));

  if (len < sizeof(unsigned int) || line == nullptr)
  {
    if (len == 0)
    {
      // when we are trying to fetch the next payload and we find out that there
      // is nothing left to read the file doesn't have to necessarily contain an
      // EOF
      payload_type = MSG_TYPE_EOF;
      return true;
    }
    return false;
  }

  payload_type = *reinterpret_cast<const unsigned int*>(line);
  return true;
}

bool read_payload_size(io_buf& input, uint32_t& payload_size)
{
  char* line = nullptr;
  auto len = input.buf_read(line, sizeof(uint32_t));
  if (len < sizeof(uint32_t) || line == nullptr) { return false; }

  payload_size = *reinterpret_cast<const uint32_t*>(line);
  return true;
}

bool read_payload(io_buf& input, char*& payload, uint32_t payload_size)
{
  char* line = nullptr;
  auto len = input.buf_read(line, payload_size);

  if (len < payload_size || line == nullptr) { return false; }
  payload = line;
  return true;
}

bool read_padding(io_buf& input, uint32_t previous_payload_size, uint32_t& padding_bytes)
{
  char* line = nullptr;
  padding_bytes = previous_payload_size % 8;
  if (padding_bytes > 0)
  {
    // read and discard padding bytes
    return read_payload(input, line, padding_bytes);
  }
  return true;
}

// helpers end

namespace VW
{
namespace external
{
binary_parser::binary_parser(std::unique_ptr<i_joiner>&& joiner, VW::io::logger logger)
    : parser(logger), _example_joiner(std::move(joiner)), _payload(nullptr), _payload_size(0), _total_size_read(0)
{
}

binary_parser::~binary_parser() {}

bool binary_parser::read_version(io_buf& input)
{
  _payload = nullptr;
  const uint32_t buffer_length = 4 * sizeof(char);
  if (!read_payload(input, _payload, buffer_length))
  {
    logger.out_critical(
        "Failed to read payload while reading file "
        "version, after having read [{}] "
        "bytes from the file",
        _total_size_read);
    return false;
  }

  _total_size_read += buffer_length;
  _payload_size = 0;  // this is used but the padding code, make it do the right thing.

  if (*_payload != BINARY_PARSER_VERSION)
  {
    logger.out_critical("File version [{}] does not match the parser version [{}]", static_cast<size_t>(*_payload),
        BINARY_PARSER_VERSION);
    return false;
  }
  return true;
}

bool binary_parser::read_header(io_buf& input)
{
  _payload = nullptr;

  // read header size
  if (!read_payload_size(input, _payload_size))
  {
    logger.out_critical(
        "Failed to read header message payload size, after having read "
        "[{}] bytes from the file",
        _total_size_read);
    return false;
  }

  _total_size_read += sizeof(_payload_size);

  // read the payload
  if (!read_payload(input, _payload, _payload_size))
  {
    logger.out_critical(
        "Failed to read header message payload of size [{}], after having read "
        "[{}] bytes from the file",
        _payload_size, _total_size_read);
    return false;
  }

  _total_size_read += _payload_size;

  // TODO:: consume header

  return true;
}

bool binary_parser::skip_over_unknown_payload(io_buf& input)
{
  _payload = nullptr;
  if (!read_payload_size(input, _payload_size))
  {
    logger.out_critical(
        "Failed to read unknown message payload size, after having read "
        "[{}] bytes from the file",
        _total_size_read);
    return false;
  }

  _total_size_read += sizeof(_payload_size);

  if (!read_payload(input, _payload, _payload_size))
  {
    logger.out_critical(
        "Failed to read unknown message payload of "
        "size [{}], after having read "
        "[{}] bytes from the file",
        _payload_size, _total_size_read);
    return false;
  }

  _total_size_read += _payload_size;

  return true;
}

bool binary_parser::read_checkpoint_msg(io_buf& input)
{
  _payload = nullptr;
  if (!read_payload_size(input, _payload_size))
  {
    logger.out_critical(
        "Failed to read checkpoint message payload size, after having read "
        "[{}] bytes from the file",
        _total_size_read);
    return false;
  }

  _total_size_read += sizeof(_payload_size);

  if (!read_payload(input, _payload, _payload_size))
  {
    logger.out_critical(
        "Failed to read reward message payload of size [{}], after having read "
        "[{}] bytes from the file",
        _payload_size, _total_size_read);
    return false;
  }

  _total_size_read += _payload_size;

  // TODO: fb verification: what if verification fails, crash or default to
  // something sensible?
  auto checkpoint_info = flatbuffers::GetRoot<v2::CheckpointInfo>(_payload);
  _example_joiner->set_reward_function(checkpoint_info->reward_function_type());
  _example_joiner->set_default_reward(checkpoint_info->default_reward());
  _example_joiner->set_learning_mode_config(checkpoint_info->learning_mode_config());
  _example_joiner->set_problem_type_config(checkpoint_info->problem_type_config());
  _example_joiner->set_use_client_time(checkpoint_info->use_client_time());

  return true;
}

bool binary_parser::read_regular_msg(io_buf& input, VW::multi_ex& examples, bool& ignore_msg)
{
  _payload = nullptr;
  ignore_msg = false;

  if (!read_payload_size(input, _payload_size))
  {
    logger.out_warn(
        "Failed to read regular message payload size, after having read "
        "[{}] bytes from the file",
        _total_size_read);
    return false;
  }

  _total_size_read += sizeof(_payload_size);

  if (!read_payload(input, _payload, _payload_size))
  {
    logger.out_warn(
        "Failed to read regular message payload of "
        "size [{}], after having read "
        "[{}] bytes from the file",
        _payload_size, _total_size_read);
    return false;
  }

  _total_size_read += _payload_size;

  if (!_example_joiner->joiner_ready())
  {
    logger.out_warn(
        "Read regular message before any checkpoint data "
        "after having read [{}] bytes from the file. Events will be ignored.",
        _total_size_read);
    ignore_msg = true;
    return true;
  }

  auto joined_payload = flatbuffers::GetRoot<v2::JoinedPayload>(_payload);
  auto verifier = flatbuffers::Verifier(reinterpret_cast<const uint8_t*>(_payload), static_cast<size_t>(_payload_size));
  if (!joined_payload->Verify(verifier))
  {
    logger.out_warn(
        "JoinedPayload of size [{}] verification failed after having read [{}] "
        "bytes from the file, skipping JoinedPayload",
        _payload_size, _total_size_read);
    return false;
  }
  _example_joiner->on_new_batch();

  for (const auto* event : *joined_payload->events())
  {
    // process and group events in batch
    if (!_example_joiner->process_event(*event))
    {
      logger.out_error(
          "Processing of an event from JoinedPayload "
          "failed after having read [{}] "
          "bytes from the file, skipping JoinedPayload",
          _total_size_read);
      return false;
    }
  }

  _example_joiner->on_batch_read();

  return process_next_in_batch(examples);
}

bool binary_parser::process_next_in_batch(VW::multi_ex& examples)
{
  while (_example_joiner->processing_batch())
  {
    if (_example_joiner->process_joined(examples)) { return true; }
    else if (!_example_joiner->current_event_is_skip_learn())
    {
      logger.out_warn(
          "Processing of a joined event from a JoinedEvent "
          "failed after having read [{}] "
          "bytes from the file, proceeding to next message",
          _total_size_read);
    }
    // else skip learn event, just process next event
  }

  // nothing form the current batch was processed, need to read next msg
  return false;
}

bool binary_parser::advance_to_next_payload_type(io_buf& input, unsigned int& payload_type)
{
  // read potential excess padding after last payload read
  uint32_t padding;
  if (!read_padding(input, _payload_size, padding))
  {
    logger.out_critical(
        "Failed to read padding of size [{}], after having read "
        "[{}] bytes from the file",
        padding, _total_size_read);
    return false;
  }

  _total_size_read += padding;

  if (!read_payload_type(input, payload_type))
  {
    logger.out_critical(
        "Failed to read next payload type from file, after having read "
        "[{}] bytes from the file",
        _total_size_read);
    return false;
  }
  _total_size_read += sizeof(payload_type);
  return true;
}

void binary_parser::persist_metrics(metric_sink& sink) { _example_joiner->persist_metrics(sink); }

bool binary_parser::parse_examples(VW::workspace*, io_buf& io_buf, VW::multi_ex& examples)
{
  if (process_next_in_batch(examples)) { return true; }

  unsigned int payload_type;
  while (advance_to_next_payload_type(io_buf, payload_type))
  {
    switch (payload_type)
    {
      case MSG_TYPE_FILEMAGIC:
      {
        if (!read_version(io_buf)) { return false; }
        break;
      }
      case MSG_TYPE_HEADER:
      {
        if (!read_header(io_buf)) { return false; }
        break;
      }
      case MSG_TYPE_CHECKPOINT:
      {
        if (!read_checkpoint_msg(io_buf)) { return false; }
        break;
      }
      case MSG_TYPE_REGULAR:
      {
        bool ignore_msg = false;
        if (read_regular_msg(io_buf, examples, ignore_msg))
        {
          if (!ignore_msg) { return true; }
        }
        break;
      }
      case MSG_TYPE_EOF:
      {
        return false;
      }

      default:
      {
        logger.out_warn(
            "Payload type not recognized [0x{:x}], after having read [{}] "
            "bytes from the file, attempting to skip payload",
            payload_type, _total_size_read);
        if (!skip_over_unknown_payload(io_buf)) { return false; }
        continue;
      }
    }
  }

  return false;
}
}  // namespace external
}  // namespace VW
