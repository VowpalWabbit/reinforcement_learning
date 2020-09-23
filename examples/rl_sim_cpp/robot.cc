#include "robot.h"
#include <utility>
#include <sstream>

joint::joint(std::string id, float temp, float ang_velocity, float load, friction_prob& outcome_probs)
: _id(std::move(id)), _temp(temp), _angular_velocity(ang_velocity),
_load(load), _outcome_probability(outcome_probs)
{}

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
    // TODO get from friction given range
    return 1.0;
}