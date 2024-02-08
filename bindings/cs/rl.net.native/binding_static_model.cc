#include "binding_static_model.h"

using namespace rl_net_native;
using namespace reinforcement_learning;

binding_static_model::binding_static_model(const char* vw_model, const size_t len) : vw_model(vw_model), len(len) {}

int binding_static_model::get_data(
    model_transport::model_data& data, reinforcement_learning::api_status* status)
{
  return data.set_data(vw_model, len);
}
