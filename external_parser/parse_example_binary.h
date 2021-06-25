// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "joiners/i_joiner.h"
#include "parse_example_external.h"

constexpr size_t BINARY_PARSER_VERSION = 1;

constexpr unsigned int MSG_TYPE_FILEMAGIC = 0x42465756; //'VWFB'
constexpr unsigned int MSG_TYPE_HEADER = 0x55555555;
constexpr unsigned int MSG_TYPE_REGULAR = 0xFFFFFFFF;
constexpr unsigned int MSG_TYPE_CHECKPOINT = 0x11111111;
constexpr unsigned int MSG_TYPE_EOF = 0xAAAAAAAA;

namespace VW {
namespace external {

class binary_parser : public parser {
public:
  binary_parser(std::unique_ptr<i_joiner>&& joiner);  //taking ownership of joiner
  ~binary_parser();
  bool parse_examples(vw *all, v_array<example *> &examples) override;
  bool read_magic(io_buf *input);
  bool read_version(io_buf *input);
  bool read_header(io_buf *input);
  bool read_checkpoint_msg(io_buf *input);
  bool read_regular_msg(io_buf *input, v_array<example *> &examples);
  bool advance_to_next_payload_type(io_buf *input, unsigned int &payload_type);
  void persist_metrics(std::vector<std::pair<std::string, size_t>>& list_metrics) override;

private:
  bool _header_read;
  std::unique_ptr<i_joiner> _example_joiner;
  char *_payload;
  uint32_t _payload_size;
  uint64_t _total_size_read;
};
} // namespace external
} // namespace VW