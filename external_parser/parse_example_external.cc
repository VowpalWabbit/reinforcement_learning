// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "joiners/example_joiner.h"
#include "joiners/multistep_example_joiner.h"
#include "parse_example_binary.h"
#include "parse_example_converter.h"
#include "utils.h"
#include "vw/core/metric_sink.h"
#include "vw/core/parse_args.h"
#include "vw/io/logger.h"

#include <cstdio>
#include <memory>

namespace VW
{
namespace external
{
std::map<const char*, v2::ProblemType> const problem_types = {{
    {"cb", v2::ProblemType_CB},
    {"ccb", v2::ProblemType_CCB},
    {"slates", v2::ProblemType_SLATES},
    {"ca", v2::ProblemType_CA},
}};

std::map<const char*, v2::RewardFunctionType> const reward_functions = {{
    {"earliest", v2::RewardFunctionType_Earliest},
    {"average", v2::RewardFunctionType_Average},
    {"median", v2::RewardFunctionType_Median},
    {"sum", v2::RewardFunctionType_Sum},
    {"min", v2::RewardFunctionType_Min},
    {"max", v2::RewardFunctionType_Max},
}};

std::map<const char*, v2::LearningModeType> const learning_modes = {{
    {"online", v2::LearningModeType_Online},
    {"apprentice", v2::LearningModeType_Apprentice},
    {"loggingonly", v2::LearningModeType_LoggingOnly},
}};

bool parser_options::is_enabled() { return binary; }

void apply_cli_overrides(std::unique_ptr<i_joiner>& joiner, VW::workspace* all, const parser_options& parsed_options)
{
  if (all->options->was_supplied("default_reward")) { joiner->set_default_reward(parsed_options.default_reward, true); }

  if (all->options->was_supplied("problem_type"))
  {
    v2::ProblemType problem_type;
    if (!str_to_enum(parsed_options.problem_type, problem_types, v2::ProblemType_UNKNOWN, problem_type))
    {
      throw std::runtime_error("Invalid argument to --problem_type " + parsed_options.problem_type);
    }
    joiner->set_problem_type_config(problem_type, true);
  }
  if (all->options->was_supplied("use_client_time"))
  {
    joiner->set_use_client_time(parsed_options.use_client_time, true);
  }
  if (all->options->was_supplied("learning_mode"))
  {
    v2::LearningModeType learning_mode;
    if (!str_to_enum(parsed_options.learning_mode, learning_modes, v2::LearningModeType_MIN, learning_mode))
    {
      throw std::runtime_error("Invalid argument to --problem_type " + parsed_options.learning_mode);
    }
    joiner->set_learning_mode_config(learning_mode, true);
  }
  if (all->options->was_supplied("reward_function"))
  {
    v2::RewardFunctionType reward_function;
    if (!str_to_enum(parsed_options.reward_function, reward_functions, v2::RewardFunctionType_MIN, reward_function))
    {
      throw std::runtime_error("Invalid argument to --problem_type " + parsed_options.reward_function);
    }
    joiner->set_reward_function(reward_function, true);
  }
  joiner->apply_cli_overrides(all, parsed_options);
}

std::unique_ptr<parser> parser::get_external_parser(VW::workspace* all, const parser_options& parsed_options)
{
  if (parsed_options.binary)
  {
    bool binary_to_json = parsed_options.binary_to_json;
    std::unique_ptr<i_joiner> joiner(nullptr);
    if (binary_to_json)
    {
      const auto& infile_path = all->data_filename;
      const auto& infile_name = infile_path.substr(0, infile_path.find_last_of('.'));
      const auto& infile_extension = infile_path.substr(infile_path.find_last_of('.') + 1);

      if (infile_extension == "dsjson")
      {
        throw std::runtime_error(
            "input file for --binary_to_json option should"
            " be binary format, file provided: " +
            infile_path);
      }

      std::string outfile_name = infile_name + ".dsjson";

      if (parsed_options.multistep)
      {
        joiner = VW::make_unique<multistep_example_joiner>(all, binary_to_json, outfile_name);
      }
      else { joiner = VW::make_unique<example_joiner>(all, binary_to_json, outfile_name); }
      apply_cli_overrides(joiner, all, parsed_options);

      return VW::make_unique<binary_json_converter>(std::move(joiner), all->logger);
    }
    else if (parsed_options.multistep) { joiner = VW::make_unique<multistep_example_joiner>(all); }
    else { joiner = VW::make_unique<example_joiner>(all); }

    apply_cli_overrides(joiner, all, parsed_options);

    if (all->options->was_supplied("extra_metrics"))
    {
      all->example_parser->metrics = VW::make_unique<VW::details::dsjson_metrics>();
    }

    return VW::make_unique<binary_parser>(std::move(joiner), all->logger);
  }
  throw std::runtime_error("external parser type not recognised");
}

void parser::set_parse_args(VW::config::option_group_definition& in_options, parser_options& parsed_options)
{
  in_options
      .add(VW::config::make_option("binary_parser", parsed_options.binary)
               .help("data file will be interpreted using the binary parser "
                     "version: " +
                   std::to_string(BINARY_PARSER_VERSION)))
      .add(VW::config::make_option("binary_to_json", parsed_options.binary_to_json)
               .help("convert binary joined log into dsjson format"))
      .add(VW::config::make_option("multistep", parsed_options.multistep).help("multistep binary joiner"))
      .add(VW::config::make_option("multistep_reward", parsed_options.multistep_reward)
               .help("Override multistep reward function to be used, valid values: suffix_mean (default), suffix_sum, "
                     "identity"))
      .add(VW::config::make_option("default_reward", parsed_options.default_reward)
               .help("Override the default reward from the file"))
      .add(VW::config::make_option("problem_type", parsed_options.problem_type)
               .help("Override the problem type trying to be solved, valid values: CB, CCB, SLATES, CA"))
      .add(VW::config::make_option("use_client_time", parsed_options.use_client_time)
               .help("Override use_client_time to define whether client time or enqueued time will be used for reward "
                     "calculation"))
      .add(VW::config::make_option("reward_function", parsed_options.reward_function)
               .help("Override the reward function to be used, valid values: earliest, average, median, sum, min, max"))
      .add(VW::config::make_option("learning_mode", parsed_options.learning_mode)
               .help("Override the learning mode from the file, valid values: Online, Apprentice, LoggingOnly"));
}

void parser::persist_metrics(metric_sink& metric_sink) { metric_sink.set_uint("external_parser", 1); }

int parse_examples(VW::workspace* all, io_buf& io_buf, VW::multi_ex& examples)
{
  bool keep_reading = all->custom_parser->next(*all, io_buf, examples);
  return keep_reading ? 1 : 0;
}

std::unique_ptr<VW::workspace> initialize_with_binary_parser(std::unique_ptr<config::options_i> options)
{
  // Add binary parser specific options
  VW::external::parser_options parsed_options;
  VW::config::option_group_definition binary_parser_config("Binary parser");
  VW::external::parser::set_parse_args(binary_parser_config, parsed_options);
  options->add_and_parse(binary_parser_config);

  auto all = VW::initialize_experimental(std::move(options));

  // Do binary parser specific setup if the binary parser is enabled.
  if (parsed_options.is_enabled())
  {
    auto external_parser = VW::external::parser::get_external_parser(all.get(), parsed_options);
    auto* external_parser_ptr = external_parser.get();
    // The metric hook will only get called if the workspace is still alive,
    // and the lifetime of the custom parser object is tied to the lifetime
    // of the workspace.
    all->global_metrics.register_metrics_callback(
        [external_parser_ptr](VW::metric_sink& metric_list) { external_parser_ptr->persist_metrics(metric_list); });
    all->custom_parser = std::unique_ptr<VW::details::input_parser>(external_parser.release());
    all->example_parser->reader = VW::external::parse_examples;
  }

  return all;
}

}  // namespace external
}  // namespace VW
