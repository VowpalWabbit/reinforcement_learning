#pragma once

#include <boost/test/unit_test.hpp>

#include "model_mgmt.h"
#include "rl_string_view.h"
#include "vw/config/options_cli.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"

#include <memory>
#include <string>

namespace reinforcement_learning
{
namespace test_utils
{
inline bool is_invoked_with(const std::string& arg)
{
  for (size_t i = 0; i < boost::unit_test::framework::master_test_suite().argc; i++)
  {
    if (reinforcement_learning::string_view(boost::unit_test::framework::master_test_suite().argv[i]).find(arg) !=
        std::string::npos)
    { return true; }
  }
  return false;
}

inline std::unique_ptr<VW::workspace> create_vw(const std::string& command_line)
{
  auto opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(VW::split_command_line(command_line)));
  return VW::initialize_experimental(std::move(opts));
}

inline std::unique_ptr<VW::workspace> create_vw(
    const std::string& command_line, const model_management::model_data& data)
{
  auto opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(VW::split_command_line(command_line)));
  auto data_reader = VW::io::create_buffer_view(data.data(), data.data_sz());
  return VW::initialize_experimental(std::move(opts), std::move(data_reader));
}

inline std::unique_ptr<VW::workspace> create_vw(const std::vector<std::string>& command_line)
{
  auto opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(command_line));
  return VW::initialize_experimental(std::move(opts));
}

inline std::unique_ptr<VW::workspace> create_vw(
    const std::vector<std::string>& command_line, const model_management::model_data& data)
{
  auto opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(command_line));
  auto data_reader = VW::io::create_buffer_view(data.data(), data.data_sz());
  return VW::initialize_experimental(std::move(opts), std::move(data_reader));
}

inline model_management::model_data save_vw(VW::workspace& vw)
{
  io_buf io_buffer;
  auto backing_buffer = std::make_shared<std::vector<char>>();
  io_buffer.add_file(VW::io::create_vector_writer(backing_buffer));
  VW::save_predictor(vw, io_buffer);

  model_management::model_data data;
  auto* data_buffer = data.alloc(backing_buffer->size());
  std::memcpy(data_buffer, backing_buffer->data(), backing_buffer->size());

  return data;
}

}  // namespace test_utils

}  // namespace reinforcement_learning
