#include "robot_joint.h"

#include <sstream>
#include <utility>

joint::joint(std::string id, float temp, float ang_velocity, float load, friction_prob& outcome_probs)
    : _id(std::move(id)), _temp(temp), _angular_velocity(ang_velocity), _load(load), _outcome_probability(outcome_probs)
{
}

std::string joint::get_features()
{
  std::ostringstream oss;
  oss << R"("id":")" << _id << R"(",)";
  oss << R"(")" << _temp << R"(":)" << 1 << R"(,)";
  oss << R"(")" << _angular_velocity << R"(":)" << 1 << R"(,)";
  oss << R"(")" << _load << R"(":)" << 1;
  return oss.str();
}

float joint::get_outcome(float observed_friction)
{
  int const draw_uniform = rand() % 10000;
  float const norm_draw_val = static_cast<float>(draw_uniform) / 100000.0f;
  float click_prob = 0.;

  // figure out which bucket from our pre-set frictions the observed_friction
  // falls into to and get it's probability
  for (auto fp : _outcome_probability)
  {
    if (observed_friction >= fp.first) { click_prob = fp.second; }
  }
  if (norm_draw_val <= click_prob) { return 1.0f; }
  return 0.0f;
}

std::string joint::id() const { return _id; }