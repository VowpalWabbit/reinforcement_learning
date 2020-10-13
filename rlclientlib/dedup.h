#pragma once
#include "api_status.h"
#include "rl_string_view.h"

#include <vector>
#include <unordered_map>


namespace reinforcement_learning {
class dedup_dict {
public:
    typedef uint64_t action_id_t;
    typedef std::vector<action_id_t> action_list_t;
    dedup_dict() = default;

    dedup_dict(const dedup_dict&) = delete;
    dedup_dict& operator=(const dedup_dict&) = delete;

    dedup_dict(dedup_dict&&) = default;
    dedup_dict& operator=(dedup_dict&&) = default;
    ~dedup_dict() = default;

    //! Returns true if the action was found. This doesn't tell the ref count status of that action
    bool remove_action(action_id_t aid);
    //! Returns the action id of the action described by [start, start+size[
    action_id_t add_action(const char* start, size_t length);
    //! Return a string_view of the action content, or an empty view if not found
    string_view get_action(action_id_t aid);

    int transform_payload(const char* payload, std::string& edited_payload, action_list_t& action_ids, api_status* status);
private:
    struct dict_entry {
        size_t _count;
        size_t _length;
        std::vector<char> _content;

        dict_entry(const char* data, size_t length);
    };
    std::unordered_map<action_id_t, dict_entry> _entries;
};

}
