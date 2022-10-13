#pragma once
#include "sender.h"

#include <fstream>

namespace reinforcement_learning
{
class i_trace;
}

namespace reinforcement_learning
{
namespace logger
{
namespace file
{
class file_logger : public i_sender
{
public:
  explicit file_logger(std::string file_name, i_trace*);
  int init(const utility::configuration& config, api_status* status) override;

  file_logger(const file_logger&) = delete;
  file_logger(file_logger&&) = delete;
  file_logger& operator=(const file_logger&) = delete;
  file_logger& operator=(file_logger&&) = delete;

protected:
  int v_send(const buffer& data, reinforcement_learning::api_status* status) override;
  std::string _file_name;
  i_trace* _trace;
  std::ofstream _file;
};
}  // namespace file
}  // namespace logger
}  // namespace reinforcement_learning
