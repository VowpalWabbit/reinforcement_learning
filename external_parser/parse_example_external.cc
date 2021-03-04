// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "parse_example_binary.h"

namespace VW {
namespace external {

std::unique_ptr<parser>
parser::get_external_parser(vw *all, const std::string &parser_type) {
  if (parser_type == "binary") {
    return VW::make_unique<binary_parser>(all);
  }
  throw std::runtime_error("parser_type specified is not recognised: " +
                           parser_type);
}

parser::~parser() {}

int parse_examples(vw *all, v_array<example *> &examples) {
  return static_cast<int>(all->external_parser->parse_examples(all, examples));
}
} // namespace external
} // namespace VW
