#include <iostream>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
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
        RETURN_ERROR_LS(trace, status, json_parse_error) << "JSON parse error: " << rj::GetParseError_En(obj.GetParseError()) << " (" << obj.GetErrorOffset() << ")";
      }

      const rj::Value::ConstMemberIterator& itr = obj.FindMember(slots);
      if (itr != obj.MemberEnd() && itr->value.IsArray()) {
        const auto& arr = itr->value.GetArray();
        for (rj::SizeType i = 0; i < arr.Size(); ++i) {
          const auto& current = arr[i];
          const auto member_itr = current.FindMember(event_id);
          if(member_itr != current.MemberEnd() && member_itr->value.IsString()) {
            const auto event_id_string = std::string(member_itr->value.GetString());
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

  struct MessageHandler : public rj::BaseReaderHandler<rj::UTF8<>, MessageHandler> {
    rj::InsituStringStream &_is;
    ContextInfo &_info;
    int _level = 0;
    int _array_level = 0;
    bool _is_multi = false;
    bool _is_slots = false;
    size_t _item_start = 0;

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
        _is_multi = !strcmp(str, multi);
        _is_slots = !strcmp(str, slots);
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
        size_t item_end = _is.Tell() - _item_start;
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
      std::ostringstream os;
      os << "JSON parse error: " << rj::GetParseError_En(res.Code()) << " (" << res.Offset() << ")";
      RETURN_ERROR_LS(trace, status, json_parse_error) << os.str();
    }
    return error_code::success;
  }
}}
