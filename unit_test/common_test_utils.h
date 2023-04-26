#pragma once

#include <boost/test/unit_test.hpp>

#include "model_mgmt.h"
#include "rl_string_view.h"
#include "vw/config/options_cli.h"
#include "vw/core/array_parameters.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/shared_data.h"
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
    {
      return true;
    }
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

inline void compare_vw(const VW::workspace& vw1, const VW::workspace& vw2)
{
  const auto sd1 = vw1.sd;
  const auto sd2 = vw2.sd;
  BOOST_CHECK_EQUAL(sd1->weighted_labeled_examples, sd2->weighted_labeled_examples);
  BOOST_CHECK_EQUAL(sd1->weighted_labels, sd2->weighted_labels);
  BOOST_CHECK_EQUAL(sd1->sum_loss, sd2->sum_loss);
  BOOST_CHECK_EQUAL(sd1->total_features, sd2->total_features);

  // These will not necessarily be equal because in trainable_vw_model
  // the binary parser will create and destroy a new example even when parsing is unsuccessful
  // BOOST_CHECK_EQUAL(sd1->example_number, sd2->example_number);
  // BOOST_CHECK_EQUAL(sd1->weighted_unlabeled_examples, sd2->weighted_unlabeled_examples);

  const auto& weights1 = vw1.weights;
  const auto& weights2 = vw2.weights;
  BOOST_CHECK_EQUAL(weights1.sparse, weights2.sparse);

  const float tolerance = 0.00001;
  if (weights1.sparse)
  {
    auto& sw1 = weights1.sparse_weights;
    auto& sw2 = weights2.sparse_weights;
    for (auto it1 = sw1.cbegin(), it2 = sw2.cbegin(); it1 != sw1.cend() && it2 != sw2.cend(); ++it1, ++it2)
    {
      BOOST_CHECK_CLOSE(*it1, *it2, tolerance);
    }
  }
  else
  {
    auto& dw1 = weights1.dense_weights;
    auto& dw2 = weights2.dense_weights;
    for (auto it1 = dw1.cbegin(), it2 = dw2.cbegin(); it1 != dw1.cend() && it2 != dw2.cend(); ++it1, ++it2)
    {
      BOOST_CHECK_CLOSE(*it1, *it2, tolerance);
    }
  }
}
}  // namespace test_utils

}  // namespace reinforcement_learning
