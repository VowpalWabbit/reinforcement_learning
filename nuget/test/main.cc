#include "live_model.h"

int main()
{
  reinforcement_learning::utility::configuration config;
  std::unique_ptr<reinforcement_learning::live_model> _rl = std::unique_ptr<reinforcement_learning::live_model>(new reinforcement_learning::live_model(config, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr));
  return 0;
}