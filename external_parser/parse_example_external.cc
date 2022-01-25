// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "parse_args.h"
#include "parse_example_binary.h"
#include "parse_example_converter.h"
#include "io/logger.h"
#include "joiners/example_joiner.h"
#include "joiners/multistep_example_joiner.h"
#include "utils.h"

#include <memory>
#include <cstdio>

namespace VW {
namespace external {

std::map<const char *, v2::ProblemType> const problem_types = {{
  { "cb", v2::ProblemType_CB },
  { "ccb", v2::ProblemType_CCB },
  { "slates", v2::ProblemType_SLATES },
  { "ca", v2::ProblemType_CA },
}};

std::map<const char *, v2::RewardFunctionType> const reward_functions = {{
  { "earliest", v2::RewardFunctionType_Earliest },
  { "average", v2::RewardFunctionType_Average },
  { "median", v2::RewardFunctionType_Median },
  { "sum", v2::RewardFunctionType_Sum },
  { "min", v2::RewardFunctionType_Min },
  { "max", v2::RewardFunctionType_Max },
}};

std::map<const char *, v2::LearningModeType> const learning_modes = {{
  { "online", v2::LearningModeType_Online },
  { "apprentice", v2::LearningModeType_Apprentice },
  { "loggingonly", v2::LearningModeType_LoggingOnly },
}};

bool parser_options::is_enabled() { return binary; }

void apply_cli_overrides(std::unique_ptr<i_joiner>& joiner, vw *all, const input_options &parsed_options)
{
  if(all->options->was_supplied("default_reward")) {
    joiner->set_default_reward(parsed_options.ext_opts->default_reward, true);
  }

  if(all->options->was_supplied("problem_type")) {
    v2::ProblemType problem_type;
    if(!str_to_enum(parsed_options.ext_opts->problem_type, problem_types, v2::ProblemType_UNKNOWN, problem_type)) {
      throw std::runtime_error("Invalid argument to --problem_type " + parsed_options.ext_opts->problem_type);
    }
    joiner->set_problem_type_config(problem_type, true);
  }
  if(all->options->was_supplied("use_client_time")) {
    joiner->set_use_client_time(parsed_options.ext_opts->use_client_time, true);
  }
  if(all->options->was_supplied("learning_mode")) {
    v2::LearningModeType learning_mode;
    if(!str_to_enum(parsed_options.ext_opts->learning_mode, learning_modes, v2::LearningModeType_MIN, learning_mode)) {
      throw std::runtime_error("Invalid argument to --problem_type " + parsed_options.ext_opts->learning_mode);
    }
    joiner->set_learning_mode_config(learning_mode, true);
  }
  if(all->options->was_supplied("reward_function")) {
    v2::RewardFunctionType reward_function;
    if(!str_to_enum(parsed_options.ext_opts->reward_function, reward_functions, v2::RewardFunctionType_MIN, reward_function)) {
      throw std::runtime_error("Invalid argument to --problem_type " + parsed_options.ext_opts->reward_function);
    }
    joiner->set_reward_function(reward_function, true);
  }
  joiner->apply_cli_overrides(all, parsed_options);
}

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
      apply_cli_overrides(joiner, all, parsed_options);

      return VW::make_unique<binary_json_converter>(std::move(joiner), all->logger);

    } else {
      joiner = VW::make_unique<example_joiner>(all);
    }
    if (parsed_options.ext_opts->multistep) {
      joiner = VW::make_unique<multistep_example_joiner>(all);
    }

    apply_cli_overrides(joiner, all, parsed_options);

    if (all->options->was_supplied("extra_metrics")) {
      all->example_parser->metrics = VW::make_unique<dsjson_metrics>();
    }

    return VW::make_unique<binary_parser>(std::move(joiner), all->logger);
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
        .help("multistep binary joiner"))
    .add(
      VW::config::make_option("multistep_reward", parsed_options.ext_opts->multistep_reward)
        .help("Override multistep reward function to be used, valid values: suffix_mean (default), suffix_sum, identity"))
    .add(
      VW::config::make_option("default_reward", parsed_options.ext_opts->default_reward)
        .help("Override the default reward from the file"))
    .add(
      VW::config::make_option("problem_type", parsed_options.ext_opts->problem_type)
        .help("Override the problem type trying to be solved, valid values: CB, CCB, SLATES, CA"))
    .add(
      VW::config::make_option("use_client_time", parsed_options.ext_opts->use_client_time)
        .help("Override use_client_time to define whether client time or enqueued time will be used for reward calculation"))
    .add(
      VW::config::make_option("reward_function", parsed_options.ext_opts->reward_function)
        .help("Override the reward function to be used, valid values: earliest, average, median, sum, min, max"))
    .add(
      VW::config::make_option("learning_mode", parsed_options.ext_opts->learning_mode)
        .help("Override the learning mode from the file, valid values: Online, Apprentice, LoggingOnly"))
    ;
}

void parser::persist_metrics(metric_sink& metric_sink) {
  metric_sink.set_uint("external_parser", 1);
}

parser::~parser() {}

int parse_examples(vw *all, io_buf& io_buf, v_array<example *> &examples) {
  return static_cast<int>(all->external_parser->parse_examples(all, io_buf, examples));
}
} // namespace external
} // namespace VW
