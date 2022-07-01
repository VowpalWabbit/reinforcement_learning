#include "live_model.h"
#include "config_utility.h"

#include <iostream>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;

int main()
{
  u::configuration config;
  auto rl = std::make_unique<r::live_model>(config);

  std::cout << "Successfully ran rlclientlib test program" << std::endl;
  return 0;
}