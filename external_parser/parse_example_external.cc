// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "parse_args.h"
#include "parse_example_binary.h"

namespace VW {
namespace external {

bool parser_options::is_enabled() { return binary; }

std::unique_ptr<parser>
parser::get_external_parser(vw *all, const input_options &parsed_options) {
  if (parsed_options.ext_opts->binary) {
    return VW::make_unique<binary_parser>(all);
  }
  throw std::runtime_error("external parser type not recognised");
}

void parser::set_parse_args(VW::config::option_group_definition &in_options,
                            input_options &parsed_options) {
  parsed_options.ext_opts = VW::make_unique<parser_options>();
  in_options.add(
      VW::config::make_option("binary_parser", parsed_options.ext_opts->binary)
          .help("data file will be interpreted using the binary parser "
                "version: " +
                std::to_string(BINARY_PARSER_VERSION)));
}

void parser::persist_metrics(std::vector<std::tuple<std::string, size_t>>& metrics) {
  metrics.emplace_back("external_parser", 1);
}

parser::~parser() {}

int parse_examples(vw *all, v_array<example *> &examples) {
  return static_cast<int>(all->external_parser->parse_examples(all, examples));
}
} // namespace external
} // namespace VW
