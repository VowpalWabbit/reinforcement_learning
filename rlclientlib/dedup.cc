#include "dedup.h"
#include "hash.h"
#include "utility/context_helper.h"

#include <sstream>

namespace reinforcement_learning
{
  namespace u = utility;

  dedup_dict::dict_entry::dict_entry(const char *data, size_t length) : _count(1),
                                                                        _length(length),
                                                                        _content(data, data + length)
  {
  }

  static dedup_dict::action_id_t hash_content(const char *start, size_t size)
  {
    return uniform_hash(start, size, 0);
  }

  dedup_dict::action_id_t dedup_dict::add_action(const char *start, size_t length)
  {
    action_id_t hash = hash_content(start, length);
    auto it = _entries.find(hash);
    if (it == _entries.end())
    {
      _entries.insert({ hash, dict_entry(start, length) });
    }
    else
    {
      ++it->second._count;
    }
    return hash;
  }

  bool dedup_dict::remove_action(action_id_t aid)
  {
    auto it = _entries.find(aid);
    if (it == _entries.end())
      return false;

    --it->second._count;
    if (!it->second._count)
      _entries.erase(it);

    return true;
  }

  string_view dedup_dict::get_action(action_id_t aid)
  {
    auto it = _entries.find(aid);
    if (it == _entries.end())
      return string_view();
    return string_view(it->second._content.data(), it->second._length);
  }

  int dedup_dict::transform_payload(const char* payload, std::string& edited_payload, action_list_t& action_ids, api_status* status)
  {
    utility::ContextInfo context_info;
    RETURN_IF_FAIL(utility::get_context_info(payload, context_info, nullptr, status));

    edited_payload = payload;
    action_ids.clear();
    action_ids.reserve(context_info.actions.size());

    int edit_offset = 0;
    for (auto &p : context_info.actions)
    {
      auto hash = add_action(&payload[p.first], p.second);
      action_ids.push_back(hash);
      std::stringstream replacement;
      replacement << "{\"__aid\":";
      replacement << hash << "}";
      edited_payload.replace(p.first - edit_offset, p.second, replacement.str());

      //adjust the edit position based on how many bytes were deleted.
      edit_offset += p.second - replacement.tellp();
    }

    return error_code::success;
  }
} // namespace reinforcement_learning
