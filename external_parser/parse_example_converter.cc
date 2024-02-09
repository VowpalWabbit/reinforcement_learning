// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "parse_example_converter.h"

#include "vw/core/example.h"
#include "vw/core/global_data.h"

namespace VW
{
namespace external
{
binary_json_converter::binary_json_converter(std::unique_ptr<i_joiner>&& joiner, const VW::io::logger& logger)
    : parser(logger), _parser(std::move(joiner), logger)
{
}

binary_json_converter::~binary_json_converter() = default;

bool binary_json_converter::parse_examples(VW::workspace* all, io_buf& io_buf, VW::multi_ex& examples)
{
  while (_parser.parse_examples(all, io_buf, examples))
  {
    // do nothing
  }
  // vw will not learn, just exit
  return false;
}

void binary_json_converter::persist_metrics(metric_sink&)
{
  // do we want metrics here?
}

}  // namespace external
}  // namespace VW