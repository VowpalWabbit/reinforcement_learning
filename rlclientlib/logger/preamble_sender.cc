#include "preamble_sender.h"

#include "api_status.h"
#include "preamble.h"

namespace reinforcement_learning
{
namespace logger
{
struct preamble;

preamble_message_sender::preamble_message_sender(i_sender* sender) : _sender{sender} {}

int preamble_message_sender::send(const uint16_t msg_type, const buffer& db, api_status* status)
{
  // Set the preamble for this message
  preamble pre;
  pre.msg_type = msg_type;
  pre.msg_size = static_cast<std::uint32_t>(db->body_filled_size());
  if (!pre.write_to_bytes(db->preamble_begin(), db->preamble_size()))
  {
    RETURN_ERROR_LS(nullptr, status, preamble_error) << " Write error.";
  }
  // Send message with preamble
  return _sender->send(db, status);
}

int preamble_message_sender::init(api_status* status) { return error_code::success; }
}  // namespace logger
}  // namespace reinforcement_learning
