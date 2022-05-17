#include "action_flags.h"
#include "generic_event.h"
#include "vw/explore/explore.h"
#include "vw/common/hash.h"

using namespace std;
namespace reinforcement_learning {
  generic_event::generic_event(const char* id, const timestamp& ts, payload_type_t type, string_view context, const char* app_id)
  : _id(id)
  , _client_time_gmt(ts)
  , _payload_type(type)
  , _app_id(app_id)
  , _context_string(context.to_string())
  {}

  generic_event::generic_event(const char* id, const timestamp& ts, payload_type_t type, flatbuffers::DetachedBuffer&& payload, event_content_type content_type, object_list_t &&objects, const char* app_id, float pass_prob)
    : _id(id)
    , _client_time_gmt(ts)
    , _payload_type(type)
    , _payload(std::move(payload))
    , _objects(std::move(objects))
    , _pass_prob(pass_prob)
    , _content_type(content_type)
    , _app_id(app_id)
    , _event_index(0) {}
  generic_event::generic_event(const char* id, const timestamp& ts, payload_type_t type, flatbuffers::DetachedBuffer&& payload, event_content_type content_type, const char* app_id, float pass_prob)
    : _id(id)
    , _client_time_gmt(ts)
    , _payload_type(type)
    , _payload(std::move(payload))
    , _pass_prob(pass_prob)
    , _content_type(content_type)
    , _app_id(app_id)
    , _event_index(0) {}
  bool generic_event::try_drop(float pass_prob, int drop_pass) {
    _pass_prob *= pass_prob;
    return prg(drop_pass) > pass_prob;
  }

  const char* generic_event::get_id() const { return _id.c_str(); }

  const char* generic_event::get_app_id() const { return _app_id.c_str(); }

  uint64_t generic_event::get_event_index() const { return _event_index; }

  void generic_event::set_event_index(uint64_t event_index) { _event_index = event_index; }

  float generic_event::get_pass_prob() const { return _pass_prob; }
  const generic_event::object_list_t& generic_event::get_object_list() const { return _objects; }

  timestamp generic_event::get_client_time_gmt() const { return _client_time_gmt; }

  float generic_event::prg(int drop_pass) const {
    const auto seed_str = _id + std::to_string(drop_pass);
    const auto seed = VW::uniform_hash(seed_str.c_str(), seed_str.length(), 0);
    return exploration::uniform_random_merand48(seed);
  }

  generic_event::payload_type_t generic_event::get_payload_type() const {
    return _payload_type;
  }

  const generic_event::payload_buffer_t& generic_event::get_payload() const {
    return _payload;
  }

  generic_event::encoding_type_t generic_event::get_encoding() const {
    switch (_content_type) {
      case event_content_type::ZSTD:
        return encoding_type_t::EventEncoding_Zstd;
      case event_content_type::IDENTITY:
      default:
        return encoding_type_t::EventEncoding_Identity;
    }
  }
}
