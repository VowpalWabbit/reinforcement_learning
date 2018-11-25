#pragma once

#include <cstdint>
#include <memory>
namespace reinforcement_learning {
  
  class api_status;

  namespace logger {
    class i_message_sender {
    public:
      using buffer = std::shared_ptr<utility::data_buffer>;
      virtual ~i_message_sender() = default;
      virtual int send(const uint16_t msg_type, buffer& db, api_status* status = nullptr) = 0;
      virtual int init(api_status* status = nullptr) = 0;
    };
  }
}
