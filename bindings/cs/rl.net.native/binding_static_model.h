#pragma once

#include "model_mgmt.h"

#include <cstring>

namespace model_transport = reinforcement_learning::model_management;

namespace rl_net_native
{

namespace constants
{
const char* const BINDING_DATA_TRANSPORT = "BINDING_DATA_TRANSPORT";
}
class binding_static_model : public model_transport::i_data_transport
{
public:
  binding_static_model(const char* vw_model, const size_t len);
  int get_data(model_transport::model_data& data,
      reinforcement_learning::api_status* status = nullptr) override;

private:
  const char* vw_model;
  const size_t len;
};
}  // namespace rl_net_native