#include "binding_static_model.h"
#include "model_mgmt.h"
#include "err_constants.h"

using namespace rl_net_native;
using namespace reinforcement_learning;

binding_static_model::binding_static_model(const char* weights, const size_t len) : weights(weights), len(len) {}

int binding_static_model::get_data(reinforcement_learning::model_management::model_data& data, reinforcement_learning::api_status* status)
{
    if(this->weights == nullptr || this->len == 0) {
        return reinforcement_learning::error_code::static_model_load_error;
    }

    char* buffer = data.alloc(this->len);
    if(buffer == nullptr) {
        return reinforcement_learning::error_code::static_model_load_error;
    }
    
    std::memcpy(buffer, this->weights, this->len);
    data.data_sz(this->len);
    
    return reinforcement_learning::error_code::success;
}
