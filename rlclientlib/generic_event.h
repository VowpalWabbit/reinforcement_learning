#pragma once
#include <string>
#include "time_helper.h"
#include "generated/v2/Event_generated.h"
#include <flatbuffers/flatbuffers.h>

namespace reinforcement_learning {
  class generic_event {
  public:
    using payload_buffer_t = flatbuffers::DetachedBuffer;
    using payload_type_t = messages::flatbuff::v2::PayloadType;

  public:
    using object_id_t = uint64_t;
    using object_list_t = std::vector<object_id_t>;

    generic_event() = default;
    generic_event(const char* id, const timestamp& ts, payload_type_t type, payload_buffer_t&& payload, object_list_t &&objects, float pass_prob = 1.f);
    generic_event(const char* id, const timestamp& ts, payload_type_t type, payload_buffer_t&& payload, float pass_prob = 1.f);

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

    payload_type_t get_payload_type() const;

    const payload_buffer_t& get_payload() const;

  protected:
    float prg(int drop_pass) const;

  protected:
    std::string _id;
    timestamp _client_time_gmt;
    payload_type_t _payload_type;
    payload_buffer_t _payload;
    object_list_t _objects;
    float _pass_prob = 1.0;
  };
}
