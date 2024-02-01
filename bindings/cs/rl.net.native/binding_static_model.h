#include <vector>

namespace rl_net_native {
class binding_static_model : public reinforcement_learning::model_management::i_data_transport {
public:
    binding_static_model(const std::vector<std::byte>& model_weights);
    int get_data(reinforcement_learning::model_management::model_data& data, reinforcement_learning::api_status* status = nullptr) override;

private:
    std::vector<std::byte> weights;
};
}