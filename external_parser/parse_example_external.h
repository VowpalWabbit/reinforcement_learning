// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/config/option_group_definition.h"
#include "vw/core/input_parser.h"
#include "vw/core/memory.h"
#include "vw/core/parse_args.h"
#include "vw/core/vw.h"

namespace VW
{
namespace external
{
struct parser_options
{
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

int parse_examples(VW::workspace* all, io_buf& io_buf, VW::multi_ex& examples);

class parser : public VW::details::input_parser
{
public:
  explicit parser(VW::io::logger logger_) : VW::details::input_parser("rl_binary"), logger(std::move(logger_)) {}
  static std::unique_ptr<parser> get_external_parser(VW::workspace* all, const parser_options& parsed_options);
  static void set_parse_args(VW::config::option_group_definition& in_options, parser_options& parsed_options);
  virtual ~parser() = default;
  virtual bool parse_examples(VW::workspace* all, io_buf& io_buf, VW::multi_ex& examples) = 0;
  virtual void persist_metrics(metric_sink& metrics);

  bool next(VW::workspace& workspace_instance, io_buf& buffer, VW::multi_ex& output_examples) override
  {
    return parse_examples(&workspace_instance, buffer, output_examples);
  }

protected:
  VW::io::logger logger;
};

std::unique_ptr<VW::workspace> initialize_with_binary_parser(std::unique_ptr<config::options_i> options);

}  // namespace external
}  // namespace VW
