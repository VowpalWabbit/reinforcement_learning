#include "binding_static_model.h"

#include "err_constants.h"
#include "model_mgmt.h"

using namespace rl_net_native;
using namespace reinforcement_learning;

binding_static_model::binding_static_model(const char* vw_model, const size_t len) : vw_model(vw_model), len(len) {}

int binding_static_model::get_data(model_transport::model_data& data, reinforcement_learning::api_status* status)
{
  if (this->vw_model == nullptr || this->len == 0)
  {
    return reinforcement_learning::error_code::static_model_load_error;
  }

  char* buffer = data.alloc(this->len);
  if (buffer == nullptr) { return reinforcement_learning::error_code::static_model_load_error; }

  memcpy(buffer, this->vw_model, this->len);
  data.data_sz(this->len);

  return reinforcement_learning::error_code::success;
}
