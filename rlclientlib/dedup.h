#pragma once
#include "api_status.h"
#include "generic_event.h"
#include "rl_string_view.h"

#include <vector>
#include <unordered_map>


namespace reinforcement_learning {
class dedup_dict {
public:
    dedup_dict() = default;

    dedup_dict(const dedup_dict&) = delete;
    dedup_dict& operator=(const dedup_dict&) = delete;

    dedup_dict(dedup_dict&&) = default;
    dedup_dict& operator=(dedup_dict&&) = default;
    ~dedup_dict() = default;

    //! Returns true if the object was found. This doesn't tell the ref count status of that object
    bool remove_object(generic_event::object_id_t aid);
    //! Returns the object id of the object described by [start, start+length[
    generic_event::object_id_t add_object(const char* start, size_t length);
    //! Return a string_view of the object content, or an empty view if not found
    string_view get_object(generic_event::object_id_t aid) const;

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

}
