#pragma once

#include "configuration.h"
#include "error_callback_fn.h"

#include <functional>
#include <string>
#include <flatbuffers/flatbuffers.h>

namespace reinforcement_learning{
// forward declare all the types
enum class event_content_type;
namespace utility { class watchdog; }
class generic_event;
class api_status;
class i_time_provider;
namespace logger { 
  template<typename TEvent, typename TFunc>
  class i_async_batcher;
  class i_message_sender;  
}

namespace logger {

    // WARNING: This interface is a bit complex in its usage. It currently lives in the live_model_impl, but is passed
    // into the interaction logger and is eventually called in the logger thread. However, it (currently) does not
    // need any form of locking because the object is ONLY called in the SINGLE logger thead.
    // The workflow looks like:
    //   live_model_impl (owner) CONTAINS interaction_logger_facade CONTAINS generic_event_logger CONTAINS
    //     async_batcher CONTROLS logger thread AND CONTAINS a queue of generic_event. generic_event will hold a pointer to this object
    // If this condition changes, then the relevant calls must be locked!
    class i_logger_extensions {
    public:
      // Due to circular dependency issues, these types are copied from generic_event
      using payload_buffer_t = flatbuffers::DetachedBuffer;
      using object_id_t = uint64_t;
      using object_list_t = std::vector<object_id_t>;
    protected:
      const utility::configuration& _config;
    public:
      i_logger_extensions(const utility::configuration&);

      virtual ~i_logger_extensions();

      virtual bool is_object_extraction_enabled() const = 0;
      virtual bool is_serialization_transform_enabled() const = 0;

      virtual i_async_batcher<generic_event, std::function<int(generic_event&, api_status*)>>* create_batcher(i_message_sender* sender, utility::watchdog& watchdog, error_callback_fn* perror_cb, const char* section) = 0;
      virtual int transform_payload_and_extract_objects(const char* context, std::string& edited_payload, object_list_t& objects, api_status* status) = 0;
      virtual int transform_serialized_payload(payload_buffer_t& input, event_content_type &content_type, api_status* status) const = 0;

      static i_logger_extensions* get_extensions(const utility::configuration& config, i_time_provider* time_provider);
    };

}}