#pragma once
#include "dedup.h"
#include "api_status.h"
#include "rl_string_view.h"
#include "zstd.h"

#include <vector>
#include <unordered_map>
#include <mutex>

namespace reinforcement_learning
{
  class dedup_dict {
  public:
    dedup_dict() = default;

    dedup_dict(const dedup_dict&) = delete;
    dedup_dict& operator=(const dedup_dict&) = delete;

    dedup_dict(dedup_dict&&) = default;
    dedup_dict& operator=(dedup_dict&&) = default;
    ~dedup_dict() = default;

    //! Returns true if the object was found. This doesn't tell the ref count status of that object
    bool remove_object(generic_event::object_id_t oid, size_t count = 1);
    //! Returns the object id of the object described by [start, start+length[
    generic_event::object_id_t add_object(const char* start, size_t length);
    //! Return a string_view of the object content, or an empty view if not found
    string_view get_object(generic_event::object_id_t oid) const;

    int transform_payload_and_add_objects(const char* payload, std::string& edited_payload, generic_event::object_list_t& object_ids, api_status* status);
  private:
    struct dict_entry {
      size_t _count;
      size_t _length;
      std::vector<char> _content;

      dict_entry(const char* data, size_t length);
    };
    std::unordered_map<generic_event::object_id_t, dict_entry> _entries;
  };

  class ewma {
  public:
    ewma(float initial = 1, float weight = 0.5): _current(initial), _weight(weight) {}

    void update(float new_value) {
      _current = (1 - _weight) * _current + (_weight * new_value);
    }

    float value() const { return _current; }
  private:
    float _current;
    const float _weight;
  };

  class zstd_compressor {
  public:
    const static int ZSTD_DEFAULT_COMPRESSION_LEVEL = 1;

    explicit zstd_compressor(int level);
    int compress(generic_event::payload_buffer_t& input, api_status* status) const;
    int decompress(generic_event::payload_buffer_t& buf, api_status* status) const;
  private:
    const int _level;
  };

  class dedup_state {
  public:
    dedup_state(const utility::configuration& c, i_time_provider* time_provider);

    string_view get_object(generic_event::object_id_t aid);
    float get_ewma_value() const;

    template<typename I>
    int get_all_values(I start, I end, generic_event::object_list_t& action_ids, std::vector<string_view>& action_values, api_status* status);

    template<typename I>
    int remove_all_values(I start, I end, api_status* status);

    void update_ewma(float value);
    int compress(generic_event::payload_buffer_t& input, api_status* status) const;
    int transform_payload_and_add_objects(const char* payload, std::string& edited_payload, generic_event::object_list_t& object_ids, api_status* status);

    i_time_provider* get_time_provider() { return _time_provider.get(); }

    //test helpers, don't use them directly
    inline dedup_dict& get_dict() { return _dict; }
    inline ewma& get_ewma() { return _ewma; }
  private:
    ewma _ewma;
    dedup_dict _dict;
    zstd_compressor _compressor;
    std::mutex _mutex;
    std::unique_ptr<i_time_provider> _time_provider;
  };

  static const char* DEDUP_DICT_EVENT_ID = "3defd95a-0122-4aac-9068-0b9ac30b66d8";
  class action_dict_builder {
  public:

    explicit action_dict_builder(dedup_state& state);

    int add(const generic_event::object_list_t& object_ids, api_status* status);
    size_t size() const;
    int finalize(generic_event& evt, api_status* status);
  private:
    dedup_state& _state;
    int _size_estimate;
    std::unordered_map<generic_event::object_id_t, size_t> _used_objects;
  };

  template<typename I>
  int dedup_state::get_all_values(I start, I end, generic_event::object_list_t& action_ids, std::vector<string_view>& action_values, api_status* status) {
    std::unique_lock<std::mutex> mlock(_mutex);

    for(; start != end; ++start) {
      auto content = _dict.get_object(start->first);
      if(content.size() == 0) {
        RETURN_ERROR_LS(nullptr, status, compression_error) << "Key not found while building batch dictionary";
      }
      action_ids.push_back(start->first);
      action_values.push_back(content);
    }

    return error_code::success;
  }

  template<typename I>
  int dedup_state::remove_all_values(I start, I end, api_status* status) {
    std::unique_lock<std::mutex> mlock(_mutex);
    for(; start != end; ++start) {
      if(!_dict.remove_object(start->first, start->second)) {
        RETURN_ERROR_LS(nullptr, status, compression_error) << "Key not found while pruning dedup_dict";
      }
    }

    return error_code::success;
  }
}
