#include "api_status.h"
#include "configuration.h"
#include "data_buffer.h"
#include "joined_log_provider.h"
#include "joiners/example_joiner.h"

#include "vw/core/global_data.h"

#include <memory>
#include <vector>

namespace reinforcement_learning
{

class vw_joined_log_batch : public i_joined_log_batch
{
public:
  vw_joined_log_batch(std::shared_ptr<VW::workspace> joiner_workspace);
  ~vw_joined_log_batch() override;

  void add_example(VW::example* example);
  // get next item in batch, outputs nullptr when empty
  virtual int next(std::unique_ptr<VW::io::reader>& chunk_reader, api_status* status = nullptr) override;
  // get next raw VW example, outputs nullptr if empty
  int next_example(VW::example** example_out, api_status* status = nullptr);
  // destructor for examples returned by next_example()
  void finish_example(VW::example* example);

private:
  VW::multi_ex _examples;
  // we need to hold a pointer to the joiner's workspace, which created the examples
  // so that finish_example can be called when we are done using the examples
  std::shared_ptr<VW::workspace> _joiner_workspace;
};

class vw_local_joiner : public i_joined_log_provider
{
  using buffer = std::shared_ptr<utility::data_buffer>;

public:
  vw_local_joiner(const utility::configuration& config, i_trace* trace_logger = nullptr, api_status* status = nullptr);
  int add_events(const std::vector<buffer>& events, api_status* status = nullptr);
  virtual int invoke_join(std::unique_ptr<i_joined_log_batch>& batch, api_status* status = nullptr) override;
  void set_trace_logger(i_trace* trace_logger);

private:
  std::unique_ptr<example_joiner> _joiner = nullptr;
  std::shared_ptr<VW::workspace> _joiner_workspace = nullptr;
  i_trace* _trace_logger = nullptr;
};

}