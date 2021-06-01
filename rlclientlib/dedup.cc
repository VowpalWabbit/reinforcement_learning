#include "dedup_internals.h"
#include "hash.h"
#include "serialization/payload_serializer.h"
#include "utility/context_helper.h"
#include "utility/config_helper.h"

#include "zstd.h"
#include <sstream>

namespace reinforcement_learning
{
namespace u = utility;
namespace fb = flatbuffers;
namespace l = reinforcement_learning::logger;

dedup_dict::dict_entry::dict_entry(const char* data, size_t length) : _count(1),
                                                                      _length(length),
                                                                      _content(data, data + length)
{
}

static generic_event::object_id_t hash_content(const char*start, size_t size)
{
  return uniform_hash(start, size, 0);
}

generic_event::object_id_t dedup_dict::add_object(const char*start, size_t length)
{
  auto hash = hash_content(start, length);
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

bool dedup_dict::remove_object(generic_event::object_id_t aid, size_t count)
{
  if (count < 1)
    return true;

  auto it = _entries.find(aid);
  if (it == _entries.end())
    return false;

  count = std::min(count, it->second._count);
  it->second._count -= count;
  if (!it->second._count)
    _entries.erase(it);

  return true;
}

string_view dedup_dict::get_object(generic_event::object_id_t aid) const
{
  auto it = _entries.find(aid);
  if (it == _entries.end())
    return string_view();
  return string_view(it->second._content.data(), it->second._length);
}

int dedup_dict::transform_payload_and_add_objects(const char* payload, std::string& edited_payload, generic_event::object_list_t& object_ids, api_status* status)
{
  u::ContextInfo context_info;
  RETURN_IF_FAIL(u::get_context_info(payload, context_info, nullptr, status));

  edited_payload = payload;
  object_ids.clear();
  object_ids.reserve(context_info.actions.size());

  size_t edit_offset = 0;
  for (auto& p : context_info.actions)
  {
    auto hash = add_object(&payload[p.first], p.second);
    object_ids.push_back(hash);
    std::stringstream replacement;
    replacement << "{\"__aid\":";
    replacement << hash << "}";
    edited_payload.replace(p.first - edit_offset, p.second, replacement.str());

    //adjust the edit position based on how many bytes were deleted.
    edit_offset += p.second - replacement.tellp();
  }

  return error_code::success;
}

size_t dedup_dict::size() const
{
  return _entries.size();
}


zstd_compressor::zstd_compressor(int level): _level(level) {}

int zstd_compressor::compress(generic_event::payload_buffer_t& input, api_status* status) const
{
  size_t buff_size = ZSTD_compressBound(input.size());

  std::unique_ptr<uint8_t[]> data(fb::DefaultAllocator().allocate(buff_size));
  size_t res = ZSTD_compress(data.get(), buff_size, input.data(), input.size(), _level);

  if(ZSTD_isError(res))
    RETURN_ERROR_ARG(nullptr, status, compression_error, ZSTD_getErrorName(res));

  auto data_ptr = data.release();
  input = fb::DetachedBuffer(nullptr, false, data_ptr, 0, data_ptr, res);
  return error_code::success;
}

int zstd_compressor::decompress(generic_event::payload_buffer_t& buf, api_status* status) const
{
  size_t buff_size = ZSTD_getFrameContentSize(buf.data(), buf.size());
  if(buff_size == ZSTD_CONTENTSIZE_ERROR)
    RETURN_ERROR_ARG(nullptr, status, compression_error, "Invalid compressed content.");
  if(buff_size == ZSTD_CONTENTSIZE_UNKNOWN)
    RETURN_ERROR_ARG(nullptr, status, compression_error, "Unknown compressed size.");

  std::unique_ptr<uint8_t[]> data(fb::DefaultAllocator().allocate(buff_size));
  size_t res = ZSTD_decompress(data.get(), buff_size, buf.data(), buf.size());

  if(ZSTD_isError(res))
    RETURN_ERROR_ARG(nullptr, status, compression_error, ZSTD_getErrorName(res));

  auto data_ptr = data.release();
  buf = fb::DetachedBuffer(nullptr, false, data_ptr, 0, data_ptr, res);
  return error_code::success;
}


dedup_state::dedup_state(const utility::configuration& c, bool use_compression, bool use_dedup, i_time_provider* time_provider):
  _compressor(c.get_int(name::ZSTD_COMPRESSION_LEVEL, zstd_compressor::ZSTD_DEFAULT_COMPRESSION_LEVEL))
  , _time_provider(time_provider)
  , _use_compression(use_compression)
  , _use_dedup(use_dedup)
{
}

string_view dedup_state::get_object(generic_event::object_id_t aid) {
  std::unique_lock<std::mutex> mlock(_mutex);
  return _dict.get_object(aid);
}

float dedup_state::get_ewma_value() const {
  return _ewma.value();
}

void dedup_state::update_ewma(float value)
{
  //This is racy, but update only reads and modifies a single value, so it won't lead to data corruption
  _ewma.update(value);
}

int dedup_state::compress(generic_event::payload_buffer_t& input, event_content_type& content_type, api_status* status) const {
  if(_use_compression) {
    content_type = event_content_type::ZSTD;
    return _compressor.compress(input, status);
  }
  content_type = event_content_type::IDENTITY;
  return error_code::success;
}

int dedup_state::transform_payload_and_add_objects(const char* payload, std::string& edited_payload, generic_event::object_list_t& object_ids, api_status* status){
  if(!_use_dedup) {
    edited_payload = payload;
    return error_code::success;
  } else {
    std::unique_lock<std::mutex> mlock(_mutex);
    return _dict.transform_payload_and_add_objects(payload, edited_payload, object_ids, status);
  }
}

action_dict_builder::action_dict_builder(dedup_state& state):
  _size_estimate(0)
  , _state(state) {}

int action_dict_builder::add(const generic_event::object_list_t& object_ids, api_status* status)
{
  for(auto aid : object_ids) {
    auto it = _used_objects.find(aid);
    if (it == _used_objects.end())
    {
      auto content = _state.get_object(aid);
      if(content.size() == 0) {
        RETURN_ERROR_LS(nullptr, status, compression_error) << "Key not found while processing event into batch dictionary";
      }
      _used_objects.insert({ aid, 1 });
      _size_estimate += sizeof(size_t) + content.size();
    }
    else
    {
      ++it->second;
    }
  }
  return error_code::success;
}

size_t action_dict_builder::size() const
{
  return (size_t)(_size_estimate * _state.get_ewma_value());
}

int action_dict_builder::finalize(generic_event& evt, api_status* status)
{
  l::dedup_info_serializer ser;
  generic_event::object_list_t action_ids;
  const auto now = _state.get_time_provider() != nullptr ? _state.get_time_provider()->gmt_now() : timestamp();
  std::vector<string_view> action_values;

  RETURN_IF_FAIL(_state.get_all_values(_used_objects.begin(), _used_objects.end(), action_ids, action_values, status));
  auto payload = ser.event(action_ids, action_values);

  //remove used actions from the dictionary
  RETURN_IF_FAIL(_state.remove_all_values(_used_objects.begin(), _used_objects.end(), status));

  //compress the payload
  event_content_type content_type;
  size_t old_size = payload.size();
  RETURN_IF_FAIL(_state.compress(payload, content_type, status));
  size_t new_size = payload.size();

  //update compression ratio estimator
  _state.update_ewma(new_size / (float)old_size);

  evt = generic_event(
    DEDUP_DICT_EVENT_ID,
    now,
    generic_event::payload_type_t::PayloadType_DedupInfo,
    std::move(payload),
    content_type);

  return error_code::success;
}

template <typename event_t>
struct dedup_collection_serializer
{
  using serializer_t = logger::fb_event_serializer<event_t>;
  using buffer_t = utility::data_buffer;
  using shared_state_t = dedup_state;

  static int message_id() { return logger::message_type::fb_generic_event_collection; }

  dedup_collection_serializer(buffer_t& buffer, const char* content_encoding, shared_state_t& state, const char* app_id)
      : _dummy(0), _ser(buffer, content_encoding, _dummy, app_id), _state(state), _builder(state) {}

  int add(event_t& evt, api_status* status = nullptr)
  {
    RETURN_IF_FAIL(_builder.add(evt.get_object_list(), status));
    return _ser.add(evt, status);
  }

  uint64_t size() const {
    return _ser.size() + _builder.size();
  }

  int finalize(api_status* status)
  {
    generic_event evt;
    RETURN_IF_FAIL(_builder.finalize(evt, status));
    RETURN_IF_FAIL(_ser.prepend(evt, status));
    RETURN_IF_FAIL(_ser.finalize(status));
    return error_code::success;
  }

  int _dummy;
  shared_state_t& _state;
  action_dict_builder _builder;
  logger::fb_collection_serializer<event_t> _ser;
};

class dedup_extensions : public logger::i_logger_extensions
{
public:
	dedup_extensions(const utility::configuration& c, bool use_compression, bool use_dedup, i_time_provider* time_provider) :
    logger::i_logger_extensions(c), _dedup_state(c, use_compression, use_dedup, time_provider), _use_dedup(use_dedup), _use_compression(use_compression) {}

	logger::i_async_batcher<generic_event>* create_batcher(logger::i_message_sender* sender, utility::watchdog& watchdog,
																									error_callback_fn* perror_cb, const char* section) override {
		auto config = utility::get_batcher_config(_config, section);

    if(_use_dedup) {
      return new logger::async_batcher<generic_event, dedup_collection_serializer>(
          sender,
          watchdog,
          _dedup_state,
          perror_cb,
          config);
    } else {
      return new logger::async_batcher<generic_event, logger::fb_collection_serializer>(
          sender,
          watchdog,
          _dummy_state,
          perror_cb,
          config);
    }

	}

  bool is_object_extraction_enabled() const override { return _use_dedup; }
  bool is_serialization_transform_enabled() const override { return _use_compression; }

	int transform_payload_and_extract_objects(const char* context, std::string& edited_payload, generic_event::object_list_t& objects, api_status* status) override {
    return _dedup_state.transform_payload_and_add_objects(context, edited_payload, objects, status);
	}

  int transform_serialized_payload(generic_event::payload_buffer_t& input, event_content_type& content_type, api_status* status) const override {
		return _dedup_state.compress(input, content_type, status);
	}
private:
	dedup_state _dedup_state;
  int _dummy_state = 0;
  bool _use_compression;
  bool _use_dedup;
};


logger::i_logger_extensions* create_dedup_logger_extension(const utility::configuration& config, const char* section, i_time_provider* time_provider) {
	if(config.get_int(name::PROTOCOL_VERSION, 1) != 2)
    return nullptr;
  const bool use_compression = config.get_bool(section, name::USE_COMPRESSION, false);
  const bool use_dedup = config.get_bool(section, name::USE_DEDUP, false);

  if(!use_compression && !use_dedup)
    return nullptr;

  return new dedup_extensions(config, use_compression, use_dedup, time_provider);
}

}
