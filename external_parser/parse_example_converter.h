// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "joiners/i_joiner.h"
#include "parse_example_binary.h"
#include "parse_example_external.h"

namespace VW
{
namespace external
{
class binary_json_converter : public parser
{
public:
  binary_json_converter(
      std::unique_ptr<i_joiner>&& joiner, const VW::io::logger& logger);  // taking ownership of joiner
  ~binary_json_converter();
  bool parse_examples(VW::workspace* all, io_buf& io_buf, VW::multi_ex& examples) override;
  void persist_metrics(metric_sink& metrics_sink) override;

private:
  binary_parser _parser;
};
}  // namespace external
}  // namespace VW