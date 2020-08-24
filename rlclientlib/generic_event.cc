#include "action_flags.h"
#include "generic_event.h"
#include "explore_internal.h"
#include "hash.h"

using namespace std;
namespace reinforcement_learning {
  generic_event::generic_event(const char* _id, const timestamp& ts, payload_type type, flatbuffers::DetachedBuffer&& payload, float pass_prob)
    : _id(_id)
    , _client_time_gmt(ts)
    , _payload_type(type)
    , _payload(std::move(payload))
    , _pass_prob(pass_prob) {}

  bool generic_event::try_drop(float pass_prob, int drop_pass) {
    _pass_prob *= pass_prob;
    return prg(drop_pass) > pass_prob;
  }

  const char* generic_event::get_id() const { return _id.c_str(); }

  float generic_event::get_pass_prob() const { return _pass_prob; }

  timestamp generic_event::get_client_time_gmt() const { return _client_time_gmt; }

  float generic_event::prg(int drop_pass) const {
    const auto seed_str = _id + std::to_string(drop_pass);
    const auto seed = uniform_hash(seed_str.c_str(), seed_str.length(), 0);
    return exploration::uniform_random_merand48(seed);
  }

  payload_type generic_event::get_payload_type() const {
    return _payload_type;
  }

  const generic_event::payload_buffer_t& generic_event::get_payload() const {
    return _payload;
  }
}
