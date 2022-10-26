#include "api_status.h"
#include "configuration.h"
#include "model_mgmt.h"
#include "vw_local_joiner.h"

#include "vw/core/global_data.h"
#include "vw/core/merge.h"

#include <memory>

namespace reinforcement_learning
{

class trainable_vw_model : public model_management::i_data_transport
{
public:
  trainable_vw_model(const utility::configuration& config);
  // overwrite internal VW model
  void set_model(std::unique_ptr<VW::workspace>&& model);
  // output current model state to buffer
  virtual int get_data(model_management::model_data& data, api_status* status = nullptr) override;
  // remove examples from joined_logs and train model on them
  int learn(vw_joined_log_batch& joined_logs, api_status* status = nullptr);
  // generate a model_delta from the current model state and the previous call to get_model_delta() or set_model()
  VW::model_delta get_model_delta();

private:
  std::string _command_line;
  // need to keep both current and starting model in order to create model_delta
  std::unique_ptr<VW::workspace> _model = nullptr;
  std::unique_ptr<VW::workspace> _starting_model = nullptr;
  void copy_current_model_to_starting();
};

}