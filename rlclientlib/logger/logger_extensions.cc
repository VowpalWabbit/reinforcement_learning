#include "logger/logger_facade.h"
#include "dedup.h"

namespace reinforcement_learning { namespace logger {

class default_extensions : public i_logger_extensions
{
public:
	default_extensions(const utility::configuration &c, i_time_provider* provider) : i_logger_extensions(c) {
		delete provider; //We don't use it
	}

	i_async_batcher<generic_event> *create_batcher(i_message_sender *sender, utility::watchdog &watchdog, error_callback_fn *perror_cb, const char *section) override {
		auto config = utility::get_batcher_config(_config, section);
		int _dummy = 0;
		return new async_batcher<generic_event, fb_collection_serializer>(
				sender,
				watchdog,
				_dummy,
				perror_cb,
				config);
	}

	bool is_enabled() override { return false; }

	int transform_payload_and_extract_objects(const char* context, std::string &edited_payload, generic_event::object_list_t &objects, api_status* status) override {
		return error_code::success;
	}

  int transform_serialized_payload(generic_event::payload_buffer_t &input, api_status* status) override {
		return error_code::success;
	}
};


i_logger_extensions::i_logger_extensions(const utility::configuration &config): _config(config) { }
i_logger_extensions::~i_logger_extensions() { }


i_logger_extensions *i_logger_extensions::get_extensions(const utility::configuration &config, i_time_provider* time_provider) {
	const bool enable_dedup = config.get_int(name::PROTOCOL_VERSION, 1) == 2 &&
		to_content_encoding_enum(config.get(name::INTERACTION_CONTENT_ENCODING, value::CONTENT_ENCODING_IDENTITY)) == content_encoding_enum::ZSTD_AND_DEDUP;

	return enable_dedup ? create_dedup_logger_extension(config, time_provider) : new default_extensions(config, time_provider);
}

} } 
