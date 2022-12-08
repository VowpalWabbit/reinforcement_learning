#include "config_utility.h"
#include "configuration.h"
#include "factory_resolver.h"
#include "live_model.h"
#include "onnx_extension.h"
#include "ranking_response.h"

#include <fstream>
#include <iostream>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;

void logging_error_fn(const r::api_status& status, void*) { std::cerr << status.get_error_msg() << std::endl; }

#define CHECK_EXIT(x)                                   \
  do                                                    \
  {                                                     \
    int retval__LINE__ = (x);                           \
    if (retval__LINE__ != 0)                            \
    {                                                   \
      std::cerr << status.get_error_msg() << std::endl; \
      std::exit(1);                                     \
    }                                                   \
  } while (0)

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    std::cerr << "USAGE: " << argv[0] << " tensor_context_data_file model_file" << std::endl;
    std::exit(1);
  }

  const std::string tensor_data_file = argv[1];
  const std::string model_file = argv[2];

  std::string JSON_CFG = R"(
  {
    "appid": "onnxtest",
    "model.implementation": "ONNXRUNTIME",
    "onnx.use_unstructured_input": true,
    "onnx.output_name": "Plus214_Output_0",
    "interaction.sender.implementation": "INTERACTION_FILE_SENDER",
    "observation.sender.implementation": "OBSERVATION_FILE_SENDER",
    "model_file_loader.file_name": ")" +
      model_file + R"(",
    "IsExplorationEnabled": true,
    "model.source": "FILE_MODEL_DATA",
    "model_file_loader.file_must_exist": true,
    "InitialExplorationEpsilon": 1.0,
    "model.backgroundrefresh": false,
    "protocol.version": 2
  }
  )";

  reinforcement_learning::onnx::register_onnx_factory();

  r::api_status status;
  u::configuration config;
  CHECK_EXIT(u::config::create_from_json(JSON_CFG, config, nullptr, &status));

  r::live_model model(config, logging_error_fn, nullptr);
  CHECK_EXIT(model.init(&status));

  std::ifstream input_data_file(tensor_data_file);
  if (!input_data_file.is_open())
  {
    std::cerr << "Failed to open: " << tensor_data_file << std::endl;
    std::exit(1);
  }

  std::string line;
  while (std::getline(input_data_file, line))
  {
    const std::string delimiter = "|";
    auto delim_it = line.find(delimiter);
    const std::string label_str = line.substr(0, delim_it);
    const std::string tensor_data_str = line.substr(delim_it + delimiter.size());

    const std::string tensor_notation_context = "{\"Input3\":" + tensor_data_str + "}";
    std::stringstream ss;
    ss << label_str;
    size_t correct_label;
    ss >> correct_label;

    r::ranking_response response;
    CHECK_EXIT(model.choose_rank(tensor_notation_context.c_str(), response, &status));

    size_t chosen_action_id;
    CHECK_EXIT(response.get_chosen_action_id(chosen_action_id, &status));

    float reward = 0.f;
    if (chosen_action_id == correct_label) { reward = 1.f; }

    std::cout << "correct: " << correct_label << ", predicted: " << chosen_action_id << ", reward: " << reward
              << std::endl;

    CHECK_EXIT(model.report_outcome(response.get_event_id(), reward, &status));
  }
}
