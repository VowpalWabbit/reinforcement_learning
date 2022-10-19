#pragma once

#include "../../rlclientlib/vw_model/vw_model.h"
#include "joiners/example_joiner.h"
#include "model_mgmt.h"
#include "sender.h"
#include "vw/core/global_data.h"

struct local_model : public reinforcement_learning::model_management::i_data_transport, reinforcement_learning::i_sender
{
  reinforcement_learning::i_trace* _trace_logger = nullptr;
  std::unique_ptr<reinforcement_learning::model_management::vw_model> _vw_model = nullptr;
  std::unique_ptr<VW::workspace> _training_workspace = nullptr;
  std::unique_ptr<example_joiner> _joiner = nullptr;
  std::mutex _mutex;

  // This needs to be cleared after a full join.
  std::vector<flatbuffers::DetachedBuffer> _detached_buffers;

  local_model() = default;
  ~local_model() override = default;

  void set_trace_logger(reinforcement_learning::i_trace* trace_logger);

  // Get data causes a "join" to take place.
  int get_data(
      reinforcement_learning::model_management::model_data& data, reinforcement_learning::api_status* status) override;

  int init(const reinforcement_learning::utility::configuration& config,
      reinforcement_learning::api_status* status) override;

  int v_send(const buffer& data, reinforcement_learning::api_status* status) override;
};

struct local_model_proxy : public reinforcement_learning::model_management::i_data_transport,
                           reinforcement_learning::i_sender
{
  explicit local_model_proxy(local_model* local_model) : _local_model(local_model) {}
  ~local_model_proxy() override = default;

  int get_data(
      reinforcement_learning::model_management::model_data& data, reinforcement_learning::api_status* status) override
  {
    return _local_model->get_data(data, status);
  }

  int init(
      const reinforcement_learning::utility::configuration& config, reinforcement_learning::api_status* status) override
  {
    return _local_model->init(config, status);
  }

protected:
  int v_send(const buffer& data, reinforcement_learning::api_status* status) override
  {
    return _local_model->v_send(data, status);
  }

private:
  local_model* _local_model;
};