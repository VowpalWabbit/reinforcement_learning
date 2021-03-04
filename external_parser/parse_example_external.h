// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "memory.h"
#include "vw.h"

namespace VW {
namespace external {

int parse_examples(vw *all, v_array<example *> &examples);

class parser {
public:
  static std::unique_ptr<parser> get_external_parser(vw *all, const std::string &parser_type);
  virtual ~parser();
  virtual bool parse_examples(vw *all, v_array<example *> &examples) = 0;
};

} // namespace external
} // namespace VW