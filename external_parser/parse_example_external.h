// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "memory.h"
#include "options.h"
#include "vw.h"

namespace VW {
namespace external {

struct parser_options {
  bool is_enabled();
  bool binary;
};

int parse_examples(vw *all, v_array<example *> &examples);

class parser {
public:
  static std::unique_ptr<parser>
  get_external_parser(vw *all, const input_options &parsed_options);
  static void set_parse_args(VW::config::option_group_definition &in_options,
                             input_options &parsed_options);
  virtual ~parser();
  virtual bool parse_examples(vw *all, v_array<example *> &examples) = 0;
};

} // namespace external
} // namespace VW