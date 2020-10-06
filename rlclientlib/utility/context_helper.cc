#include <iostream>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <object_factory.h>
#include "err_constants.h"
#include "utility/context_helper.h"

#include <chrono>
#include <cstring>

namespace reinforcement_learning { namespace utility {
  namespace rj = rapidjson;

  const auto multi = "_multi";
  const auto slots = "_slots";
  const auto event_id = "_id";

  /**
   * \brief Get the event IDs from the slots entries in the context json string.
   * 
   * \param context   : String with context json
   * \param event_ids : Reference to the mapping from slot index to event ID string, results will be
   *                    put in here
   * \param trace     : Pointer to the trace logger
   * \param status    : Pointer to api_status object that contains an error code and error description in
   *                    case of failure
   * \return  error_code::success if there are no errors.  If there are errors then the error code is
   *          returned.
   */
  int get_event_ids(const char* context, std::map<size_t, std::string>& event_ids, i_trace* trace, api_status* status) {
    try {
      rj::Document obj;
      obj.Parse(context);

      if (obj.HasParseError()) {
        RETURN_ERROR_LS(trace, status, json_parse_error) << "JSON parse error: " << rj::GetParseErrorFunc(obj.GetParseError()) << " (" << obj.GetErrorOffset() << ")";
      }

      const rj::Value::ConstMemberIterator& itr = obj.FindMember(slots);
      if (itr != obj.MemberEnd() && itr->value.IsArray()) {
        const auto& arr = itr->value.GetArray();
        for (int i = 0; i < arr.Size(); ++i) {
          const auto& current = arr[i];
          const auto itr = current.FindMember(event_id);
          if(itr != current.MemberEnd() && itr->value.IsString()) {
            const auto event_id_string = std::string(itr->value.GetString());
            event_ids[i] = std::string{ event_id_string.begin(), event_id_string.end() };
          }
        }

        return error_code::success;
      }
      RETURN_ERROR_LS(trace, status, json_no_slots_found);
    }
    catch ( const std::exception& e ) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << error_code::unknown_s;
    }
  }

  /**
   * \brief Get the number of elements in the given array found in context json string.
   *
   * \param context     : String with context json
   * \param array_key   : String with the key for the array in the context json
   * \param trace       : Pointer to the trace logger
   * \param status      : Pointer to api_status object that contains an error code and error description in
   *                      case of failure
   * \param parse_error : The parse error string. Will be empty if no parse error exists.
   * \return  the number of elements in the array. If the array is empty or if the key could not be found
   *          then a 0 is returned. If there was a parse error, a -1 is returned;
   */
  int get_array_count(const char *context, const char *array_key, i_trace *trace, api_status *status, std::string& parse_error) {
    auto count = 0;
    try {
      rj::Document obj;
      obj.Parse(context);

      if (obj.HasParseError()) {
        std::ostringstream ss;
        ss << "JSON parse error: " << rj::GetParseErrorFunc(obj.GetParseError()) << " (" << obj.GetErrorOffset() << ")";
        parse_error = ss.str();
      }
      else {
        const rj::Value::ConstMemberIterator itr = obj.FindMember(array_key);
        if (itr != obj.MemberEnd() && itr->value.IsArray()) {
          const auto arr = itr->value.GetArray();
          count = arr.Size();
        }
      }
    }
    catch ( const std::exception& e ) {
      parse_error = e.what();
    }
    catch ( ... ) {
      parse_error = error_code::unknown_s;
    }

    return count;
  }

  /**
   * \brief Get the number of actions found in context json string.  Actions should be in an array
   * called _multi under the root name space.  {_multi:[{"name1":"val1"},{"name1":"val1"}]}
   *
   * \param count   : Return value passed in as a reference.
   * \param context : String with context json
   * \param trace   : Pointer to the trace logger
   * \param status  : Pointer to api_status object that contains an error code and error description in
   *                  case of failure
   * \return  error_code::success if there are no errors.  If there are errors then the error code is
   *          returned.
   */
  int get_action_count(size_t& count, const char *context, i_trace* trace, api_status* status) {
    std::string parse_error;
    count = get_array_count(context, multi, trace, status, parse_error);
    if (!parse_error.empty()) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << parse_error;
    }

    if (count > 0) {
      return error_code::success;
    }

    RETURN_ERROR_LS(trace, status, json_no_actions_found);
  }

  /**
   * \brief Get the number of slots found in context json string.  Slots should be in an array
   * called _slots under the root name space.  {_slots:[{"name1":"val1"},{"name1":"val1"}]}
   *
   * \param count   : Return value passed in as a reference.
   * \param context : String with context json
   * \param trace   : Pointer to the trace logger
   * \param status  : Pointer to api_status object that contains an error code and error description in
   *                  case of failure
   * \return  error_code::success if there are no errors.  If there are errors then the error code is
   *          returned.
   */
  int get_slot_count(size_t& count, const char *context, i_trace* trace, api_status* status) {
    std::string parse_error;
    count = get_array_count(context, slots, trace, status, parse_error);
    if (!parse_error.empty()) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << parse_error;
    }

    if ( count > 0 ) {
      return error_code::success;
    }

    RETURN_ERROR_LS(trace, status, json_no_slots_found);
  }

  /**
   * \brief Validate that the _multi and _slots entries exist in the context json string,
   * and that _multi comes before _slots
   * 
   * \param context : String with context json
   * \param trace   : Pointer to the trace logger
   * \param status  : Pointer to api_status object that contains an error code and error description in
   *                  case of failure
   * \return  error_code::success if there are no errors.  If there are errors then the error code is
   *          returned.
   */
  int validate_multi_before_slots(const char *context, i_trace* trace, api_status* status)
  {
    auto slots_pos = strstr(context, "_slots");
    auto multi_pos = strstr(context, "_multi");

    if(slots_pos != nullptr && multi_pos != nullptr && slots_pos < multi_pos)
    {
      RETURN_ERROR_LS(trace, status, json_parse_error) << " There must be both a _multi field and _slots, and _multi must come first.";
    }

    return reinforcement_learning::error_code::success;
  }

  struct MessageHandler : public rj::BaseReaderHandler<rj::UTF8<>, MessageHandler> {
    rj::InsituStringStream &_is;
    ContextInfo &_info;
    int _level;
    int _array_level;
    bool _is_multi;
    bool _is_slots;
    size_t _item_start;

    MessageHandler(rj::InsituStringStream &is, ContextInfo &info) : 
      _is(is),
      _info(info),
      _level(0),
      _array_level(0),
      _is_multi(false),
      _item_start(0)
       { }

    bool Key(const char* str, size_t length, bool copy)
    {
      if(_level == 1 && _array_level == 0) {
        _is_multi = !strcmp(str, "_multi");
        _is_slots = !strcmp(str, "_slots");
      }
      return true;
    }

    bool StartObject()
    {
      if((_is_multi | _is_slots) && _level == 1 && _array_level == 1)
        _item_start = _is.Tell() - 1;

      ++_level;
      return true;
    }

    bool EndObject(rj::SizeType memberCount)
    {
      --_level;

      if((_is_multi | _is_slots) && _level == 1 && _array_level == 1) {
        size_t item_end = _is.Tell() - 1;
        if(_is_multi)
          _info.actions.push_back(std::make_pair(_item_start, item_end));
        if(_is_slots)
          _info.slots.push_back(std::make_pair(_item_start, item_end));
      }
      return true;
    }

    bool StartArray()
    {
      ++_array_level;
      return true;
    }

    bool EndArray(rj::SizeType elementCount)
    {
      --_array_level;
      return true;
    }
  };

  int get_context_info(const char *context, ContextInfo &info, i_trace* trace, api_status* status)
  {
    std::string copy(context);
    info.actions.clear();
    info.slots.clear();

    rj::InsituStringStream iss((char*)copy.c_str());
    MessageHandler mh(iss, info);

    rj::Reader reader;
    auto res = reader.Parse<rj::kParseInsituFlag>(iss, mh);
    if(res.IsError()) {
        RETURN_ERROR_LS(trace, status, json_parse_error) << "JSON parse error: " << rj::GetParseErrorFunc(res.Code()) << " (" << res.Offset() << ")";
    }
    return error_code::success;
  }
}}
