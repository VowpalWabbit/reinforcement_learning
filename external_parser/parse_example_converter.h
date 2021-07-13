// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "joiners/i_joiner.h"
#include "parse_example_external.h"
#include "parse_example_binary.h"

namespace VW {
namespace external {

class binary_json_converter : public parser {
public:
  binary_json_converter(std::unique_ptr<i_joiner>&& joiner);  //taking ownership of joiner
  ~binary_json_converter();
  bool parse_examples(vw *all, v_array<example *> &examples) override;
  void persist_metrics(std::vector<std::pair<std::string, size_t>>& list_metrics) override;

private:
  binary_parser _parser;
};
} // namespace external
} // namespace VW