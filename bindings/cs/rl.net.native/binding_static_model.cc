#include "binding_static_model.h"

using namespace rl_net_native;

binding_static_model::binding_static_model(const std::vector<std::byte>& model_weights) : weights(model_weights) {
}

int binding_static_model::get_data(reinforcement_learning::model_management::model_data& data, reinforcement_learning::api_status* status) {
    data.set_data(weights.data(), weights.size());

    return error_code::success;
}
