// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "parse_args.h"
#include "parse_example_binary.h"
#include "io/logger.h"
#include "example_joiner.h"
#include "multistep_example_joiner.h"

#include <memory>
#include <cstdio>


namespace VW {
namespace external {

bool parser_options::is_enabled() { return binary; }

std::unique_ptr<parser>
parser::get_external_parser(vw *all, const input_options &parsed_options) {
  if (parsed_options.ext_opts->binary) {
    bool binary_to_json = parsed_options.ext_opts->binary_to_json;
    std::unique_ptr<i_joiner> joiner(nullptr);
    if (binary_to_json) {
      const auto& infile_path = all->data_filename;
      const auto& infile_name = infile_path.substr(
        0, infile_path.find_last_of('.'));
      const auto& infile_extension = infile_path.substr(
        infile_path.find_last_of(".") + 1);

      if (infile_extension == "dsjson") {
        throw std::runtime_error("input file for --binary_to_json option should"
        " be binary format, file provided: " + infile_path);
      }

      std::string outfile_name = infile_name + ".dsjson";
      joiner = VW::make_unique<example_joiner>(all, binary_to_json, outfile_name);
    } else {
      joiner = VW::make_unique<example_joiner>(all);
    }
    if (parsed_options.ext_opts->multistep) {
      joiner = VW::make_unique<multistep_example_joiner>(all);
    }
    return VW::make_unique<binary_parser>(std::move(joiner));
  }
  throw std::runtime_error("external parser type not recognised");
}

void parser::set_parse_args(VW::config::option_group_definition &in_options,
                            input_options &parsed_options) {
  parsed_options.ext_opts = VW::make_unique<parser_options>();
  in_options
    .add(
      VW::config::make_option("binary_parser", parsed_options.ext_opts->binary)
        .help("data file will be interpreted using the binary parser "
              "version: " +
              std::to_string(BINARY_PARSER_VERSION)))
    .add(
      VW::config::make_option("binary_to_json", parsed_options.ext_opts->binary_to_json)
        .help("convert binary joined log into dsjson format"))
    .add(
      VW::config::make_option("multistep", parsed_options.ext_opts->multistep)
        .help("multistep binary joiner"));
}

void parser::persist_metrics(std::vector<std::pair<std::string, size_t>>& metrics) {
  metrics.emplace_back("external_parser", 1);
}

parser::~parser() {}

int parse_examples(vw *all, v_array<example *> &examples) {
  return static_cast<int>(all->external_parser->parse_examples(all, examples));
}
} // namespace external
} // namespace VW
