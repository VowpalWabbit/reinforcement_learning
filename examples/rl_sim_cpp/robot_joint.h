#pragma once
#include <map>
#include <string>

class joint
{
public:
  // Collection type of outcome probability for a given friction range
  using friction_prob = std::map<float, float>;
  joint(std::string id, float temp, float ang_velocity, float load, friction_prob& outcome_probs);

  std::string get_features();
  float get_outcome(float observed_friction);
  std::string id() const;

private:
  const std::string _id;
  const float _temp;
  const float _angular_velocity;
  const float _load;
  friction_prob _outcome_probability;
};