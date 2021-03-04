// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "example_joiner.h"
#include "parse_example_external.h"

const unsigned int MSG_TYPE_HEADER = 0x55555555;
const unsigned int MSG_TYPE_REGULAR = 0xFFFFFFFF;
const unsigned int MSG_TYPE_EOF = 0xAAAAAAAA;

namespace VW {
namespace external {

class binary_parser : public parser {
public:
  binary_parser(vw *all);
  ~binary_parser();
  bool parse_examples(vw *all, v_array<example *> &examples) override;

private:
  bool _header_read = false;
  example_joiner _example_joiner;
};
} // namespace external
} // namespace VW