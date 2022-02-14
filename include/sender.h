#pragma once
#include "configuration.h"
#include "data_buffer.h"
#include <memory>
namespace reinforcement_learning {
  class api_status;
  class i_sender {
  public:
    using buffer = std::shared_ptr<utility::data_buffer>;
    virtual int init(const utility::configuration& config, api_status* status) = 0;
    int send(const buffer& data, api_status* status = nullptr) { return v_send(data, status); }
    int send(const buffer& data, unsigned int number_of_events, api_status* status = nullptr) { return v_send(data, number_of_events,status); }
    virtual ~i_sender() = default;
  protected:
    virtual int v_send(const buffer& data, api_status* status = nullptr) = 0;
    virtual int v_send(const buffer& data, unsigned int number_of_events, api_status* status = nullptr) {
        return v_send(data, status);
    }
  };
} // namespace reinforcement_learning
