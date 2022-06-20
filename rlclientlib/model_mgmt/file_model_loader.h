#pragma once
#include "model_mgmt.h"
namespace reinforcement_learning
{
class i_trace;
}

namespace reinforcement_learning
{
namespace model_management
{

class file_model_loader : public i_data_transport
{
public:
  file_model_loader(std::string file_name, bool file_must_exist, i_trace* trace_logger);
  int init(api_status* status = nullptr);
  int get_data(model_data& data, api_status* status = nullptr) override;

private:
  int get_file_modified_time(time_t& file_time, api_status* status) const;

private:
  std::string _file_name;
  bool _file_must_exist;
  i_trace* _trace;
  time_t _last_modified = 0;
  size_t _datasz{};
};

}  // namespace model_management
}  // namespace reinforcement_learning
