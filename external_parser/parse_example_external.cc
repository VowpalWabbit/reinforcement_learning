// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "parse_args.h"
#include "parse_example_binary.h"
#include <stdio.h>
#include "io/logger.h"

namespace VW {
namespace external {

bool parser_options::is_enabled() { return binary; }

std::unique_ptr<parser>
parser::get_external_parser(vw *all, const input_options &parsed_options) {
  if (parsed_options.ext_opts->binary) {
    bool binary_to_json = parsed_options.ext_opts->binary_to_json;
    if (binary_to_json) {
      std::string infile_path = all->data_filename;
      std::string infile_name = infile_path.substr(
        0, infile_path.find_last_of('.'));
      std::string infile_extension = infile_path.substr(
        infile_path.find_last_of(".") + 1);

      if (infile_extension == "dsjson") {
        throw std::runtime_error("input file for --binary_to_json option should"
        " be binary format");
      }

      std::string outfile_name = infile_name + ".dsjson";
      return VW::make_unique<binary_parser>(all, binary_to_json, outfile_name);
    }
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
  in_options.add(
    VW::config::make_option("binary_to_json", parsed_options.ext_opts->binary_to_json)
      .help("convert binary joined log into dsjson format"));
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
