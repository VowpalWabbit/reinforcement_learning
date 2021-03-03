// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "example_joiner.h"
#include "vw.h"

const unsigned int MSG_TYPE_HEADER = 0x55555555;
const unsigned int MSG_TYPE_REGULAR = 0xFFFFFFFF;
const unsigned int MSG_TYPE_EOF = 0xAAAAAAAA;
namespace VW {

int parse_examples(vw *all, v_array<example *> &examples);

class external_parser {
public:
  external_parser(vw *all);
  bool parse_examples(vw *all, v_array<example *> &examples);

private:
  bool _header_read = false;
  ExampleJoiner _example_joiner;
};
} // namespace VW