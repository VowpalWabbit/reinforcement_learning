#include "multistep.h"

#include "api_status.h"
#include "err_constants.h"

#define RAPIDJSON_HAS_STDSTRING 1

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace reinforcement_learning
{
namespace rj = rapidjson;

void episode_history::update(
    const char* event_id, const char* previous_event_id, string_view /*context*/, const ranking_response& /*resp*/)
{
  _depths[event_id] = this->get_depth(previous_event_id) + 1;
}

std::string episode_history::get_context(const char* previous_event_id, string_view context) const
{
  return R"({"episode":{"depth":")" + std::to_string(this->get_depth(previous_event_id) + 1) + "\"}," +
      std::string(context.data() + 1);
}

int episode_history::get_depth(const char* id) const
{
  if (id == nullptr) { return 0; }
  auto result = _depths.find(id);
  return (result == _depths.end()) ? 0 : result->second;
}

size_t episode_history::size() const { return _depths.size(); }

std::string episode_history::dump_to_json() const
{
  rj::Document doc;
  doc.SetObject();
  auto& allocator = doc.GetAllocator();

  for (const auto& event_id_depth : _depths)
  {
    doc.AddMember(rapidjson::StringRef(event_id_depth.first), event_id_depth.second, allocator);
  }

  rj::StringBuffer buffer;
  rj::Writer<rj::StringBuffer> writer(buffer);
  doc.Accept(writer);

  return buffer.GetString();
}

int episode_history::init_from_json(const char* history_json, api_status* status)
{
  constexpr auto error_message = "\nFailed to initialize episode history.";

  rj::Document doc;
  doc.Parse(history_json);
  if (doc.HasParseError())
  {
    RETURN_ERROR_LS(nullptr, status, json_parse_error)
        << "JSON parse error: " << rj::GetParseError_En(doc.GetParseError()) << " (" << doc.GetErrorOffset() << ")"
        << error_message;
  }

  for (const auto& name_value : doc.GetObject())
  {
    const char* event_id = name_value.name.GetString();
    const auto& depth = name_value.value;
    if (!depth.IsInt())
    {
      RETURN_ERROR_LS(nullptr, status, json_parse_error)
          << "Invalid json type found: " << depth.GetType() << error_message;
    }
    else { _depths[event_id] = depth.GetInt(); }
  }

  return error_code::success;
}

episode_state::episode_state(const char* episode_id) : _episode_id(episode_id) {}

const char* episode_state::get_episode_id() const { return _episode_id.c_str(); }

const episode_history& episode_state::get_history() const { return _history; }

size_t episode_state::size() const { return _history.size(); }

std::string episode_state::dump_history_to_json() const { return _history.dump_to_json(); }

int episode_state::init_history_from_json(const char* history_json, api_status* status)
{
  return _history.init_from_json(history_json, status);
}

int episode_state::update(const char* event_id, const char* previous_event_id, string_view context,
    const ranking_response& response, api_status* /*status*/)
{
  _history.update(event_id, previous_event_id, context, response);
  return error_code::success;
}
}  // namespace reinforcement_learning
