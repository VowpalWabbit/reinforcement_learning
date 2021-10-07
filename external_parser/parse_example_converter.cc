// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "parse_example_converter.h"
#include "example.h"
#include "global_data.h"

namespace VW {
namespace external {

binary_json_converter::binary_json_converter(std::unique_ptr<i_joiner> &&joiner)
    : _parser(std::move(joiner)) {}

binary_json_converter::~binary_json_converter() = default;

bool binary_json_converter::parse_examples(vw *all, io_buf &io_buf,
                                           v_array<example *> &examples) {
  while (_parser.parse_examples(all, io_buf, examples)) {
    // do nothing
  }
  // vw will not learn, just exit
  return false;
}

void binary_json_converter::persist_metrics(
    std::vector<std::pair<std::string, size_t>> &) {
  // do we want metrics here?
}

} // namespace external
} // namespace VW