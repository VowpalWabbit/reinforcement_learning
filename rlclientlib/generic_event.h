#pragma once
#include <string>
#include "api_status.h"
#include "time_helper.h"
#include "generated/v2/Event_generated.h"
#include "logger/logger_extensions.h"
#include <flatbuffers/flatbuffers.h>

namespace reinforcement_learning {
  namespace logger { class i_logger_extensions; }
  enum class event_content_type {
    IDENTITY,
    ZSTD
  };

  class generic_event {
  public:
    using payload_buffer_t = flatbuffers::DetachedBuffer;
    using payload_type_t = messages::flatbuff::v2::PayloadType;
    using encoding_type_t = messages::flatbuff::v2::EventEncoding;
  public:
    using object_id_t = uint64_t;
    using object_list_t = std::vector<object_id_t>;

    generic_event() = default;
    generic_event(const char* id, const timestamp& ts, payload_type_t type, string_view context, const char* app_id);
    generic_event(const char* id, const timestamp& ts, payload_type_t type, payload_buffer_t&& payload, event_content_type content_type, object_list_t &&objects, const char* app_id, float pass_prob = 1.f);
    generic_event(const char* id, const timestamp& ts, payload_type_t type, payload_buffer_t&& payload, event_content_type content_type, const char* app_id, float pass_prob = 1.f);

    generic_event(const generic_event&) = delete;
    generic_event& operator=(const generic_event&) = delete;

    generic_event(generic_event&&) = default;
    generic_event& operator=(generic_event&&) = default;
    ~generic_event() = default;

    float get_pass_prob() const;
    timestamp get_client_time_gmt() const;
    bool try_drop(float pass_prob, int drop_pass);

    const object_list_t& get_object_list() const;

    const char* get_id() const;

    const char* get_app_id() const;

    uint64_t get_event_index() const;

    void set_event_index(uint64_t event_index);

    payload_type_t get_payload_type() const;

    const payload_buffer_t& get_payload() const;

    encoding_type_t get_encoding() const;
    
    // context_string is only valid before the event is transformed
    const std::vector<uint8_t>& get_context_string() const {
      return _context_string;
    }

    // generate a serializable event
    // This only works with a context string, other event types cannot be transformed
    template<typename TSerializer, typename... Args>
    int transform(logger::i_logger_extensions* ext, TSerializer& serializer, api_status* status, Args... args) {
      const char* data = reinterpret_cast<const char*>(get_context_string().data());
      if(!ext->is_object_extraction_enabled()) {
        _payload = serializer.event(data, args...);
      } else {
        std::string tmp;
        RETURN_IF_FAIL(ext->transform_payload_and_extract_objects(data, tmp, _objects, status));
        _payload = serializer.event(tmp.c_str(), args...);
      }
      if(ext->is_serialization_transform_enabled()) {
        RETURN_IF_FAIL(ext->transform_serialized_payload(_payload, _content_type, status));
      } else {
        _content_type = event_content_type::IDENTITY;
      }
     _context_string.clear(); 
     return 0;
    }
  protected:
    float prg(int drop_pass) const;

  protected:
    std::string _id;
    timestamp _client_time_gmt;
    payload_type_t _payload_type;
    payload_buffer_t _payload;
    object_list_t _objects;
    float _pass_prob = 1.0;
    event_content_type _content_type;
    std::string _app_id;
    uint64_t _event_index;
    std::vector<unsigned char> _context_string;
  };
}
