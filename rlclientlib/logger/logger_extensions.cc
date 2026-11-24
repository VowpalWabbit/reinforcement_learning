#include "dedup.h"
#include "logger/logger_facade.h"

namespace reinforcement_learning
{
namespace logger
{
class default_extensions : public i_logger_extensions
{
public:
  default_extensions(const utility::configuration& c, std::unique_ptr<i_time_provider> /* provider */)
      : i_logger_extensions(c)
  {
    // i_time_provider is not used
    // unique_ptr will delete it
  }

  std::unique_ptr<i_async_batcher<generic_event>> create_batcher(std::unique_ptr<i_message_sender> sender,
      utility::watchdog& watchdog, error_callback_fn* perror_cb, const char* section) override
  {
    auto config = utility::get_batcher_config(_config, section);
    return std::unique_ptr<async_batcher<generic_event, fb_collection_serializer>>(
        new async_batcher<generic_event, fb_collection_serializer>(
            std::move(sender), watchdog, _dummy_state, perror_cb, config));
  }

  bool is_object_extraction_enabled() const override { return false; }
  bool is_serialization_transform_enabled() const override { return false; }

  int transform_payload_and_extract_objects(string_view context, std::string& edited_payload,
      generic_event::object_list_t& objects, api_status* status) override
  {
    return error_code::success;
  }

  int transform_serialized_payload(
      generic_event::payload_buffer_t& input, event_content_type& content_type, api_status* status) const override
  {
    content_type = event_content_type::IDENTITY;
    return error_code::success;
  }

private:
  int _dummy_state = 0;
};

i_logger_extensions::i_logger_extensions(const utility::configuration& config) : _config(config) {}
i_logger_extensions::~i_logger_extensions() = default;

std::unique_ptr<i_logger_extensions> i_logger_extensions::get_extensions(
    const utility::configuration& config, std::unique_ptr<i_time_provider> time_provider)
{
  const char* section = "interaction";  // fixme lift this to live_model_impl;
  if (should_use_dedup_logger_extension(config, section))
  {
    return create_dedup_logger_extension(config, section, std::move(time_provider));
  }
  return std::unique_ptr<i_logger_extensions>(new default_extensions(config, std::move(time_provider)));
}

}  // namespace logger
}  // namespace reinforcement_learning
