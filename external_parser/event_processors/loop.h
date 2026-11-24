#pragma once

// FileFormat_generated.h used for the payload type and encoding enum's
#include "generated/v2/FileFormat_generated.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace loop
{
template <typename T>
class sticky_value
{
  bool _sticky;  // once a value is set to sticky, it cannot be changed by further calls to set
  bool _set;
  T _value;

public:
  sticky_value() : _sticky(false), _set(false), _value() {}
  explicit sticky_value(T value) : _sticky(false), _set(true), _value(value) {}
  void set(T value, bool sticky = false)
  {
    if (!_sticky)
    {
      _sticky = sticky;
      _value = value;
      _set = true;
    }
  }

  // this should be used with trivial & small types, so copying is fine
  T value() const { return _value; }

  bool is_valid() const { return _set; }
  bool is_sticky() const { return _sticky; }

  operator T() const { return _value; }
};

struct loop_info
{
  sticky_value<float> default_reward = sticky_value<float>(0.f);
  sticky_value<v2::LearningModeType> learning_mode_config =
      sticky_value<v2::LearningModeType>(v2::LearningModeType_Online);
  sticky_value<v2::ProblemType> problem_type_config;
  sticky_value<bool> use_client_time = sticky_value<bool>(false);

  bool is_configured() const
  {
    return default_reward.is_valid() && learning_mode_config.is_valid() && problem_type_config.is_valid() &&
        use_client_time.is_valid();
  }  //&& type.is_valid()
};
}  // namespace loop