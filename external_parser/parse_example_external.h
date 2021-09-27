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
  bool binary_to_json;
  bool multistep;
  float default_reward;
  std::string multistep_reward;
  std::string problem_type;
  std::string reward_function;
  std::string learning_mode;
  bool use_client_time;
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
  virtual void persist_metrics(std::vector<std::pair<std::string, size_t>>& list_metrics);
};

} // namespace external
} // namespace VW
