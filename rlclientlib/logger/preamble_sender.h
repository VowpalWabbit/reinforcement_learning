#include "data_buffer.h"
#include "message_sender.h"
#include "sender.h"

namespace reinforcement_learning { namespace logger {
    class preamble_message_sender : public i_message_sender {
    public:
      explicit preamble_message_sender(i_sender*);
      int send(const uint16_t msg_type, const buffer& db, api_status* status) override;
      int init(api_status* status) override;
    private:
      std::unique_ptr<i_sender> _sender;
    };
}}
