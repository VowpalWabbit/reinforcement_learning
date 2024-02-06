#pragma once

#include "model_mgmt.h"

namespace rl_net_native {

namespace constants
{
const char* const BINDING_DATA_TRANSPORT = "BINDING_DATA_TRANSPORT";
}
class binding_static_model : public reinforcement_learning::model_management::i_data_transport {
public:
    binding_static_model(const char* weights, const size_t len);
    int get_data(reinforcement_learning::model_management::model_data& data, reinforcement_learning::api_status* status = nullptr) override;

private:
    const char* weights;
    const size_t len;
};
}