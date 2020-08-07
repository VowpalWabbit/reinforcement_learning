#pragma once

#pragma once

#include <vector>

#include "ranking_event.h"
#include "data_buffer.h"
#include "logger/message_type.h"
#include "api_status.h"
#include "utility/data_buffer_streambuf.h"

namespace reinforcement_learning {
  namespace logger {

    template<payload_type pt>
    struct payload_serializer {
      const payload_type type = pt;
    };

    struct cb_serializer : payload_serializer<payload_type::CB> {
      static std::vector<unsigned char>&& event(const char* context, unsigned int flags, const ranking_response& response) {
        return std::move(std::vector<unsigned char>());
      }
    };

    struct ccb_serializer : payload_serializer<payload_type::CCB> {
      static std::vector<unsigned char>&& event(const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
        const std::vector<std::vector<float>>& pdfs, const std::string& model_version) {
        return std::move(std::vector<unsigned char>());
      }
    };

    struct slates_serializer : payload_serializer<payload_type::SLATES> {
      static std::vector<unsigned char>&& event(const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
        const std::vector<std::vector<float>>& pdfs, const std::string& model_version) {
        return std::move(std::vector<unsigned char>());
      }
    };

    struct outcome_single_serializer : payload_serializer<payload_type::OUTCOME_SINGLE> {
      static std::vector<unsigned char>&& event(float outcome) {
        return std::move(std::vector<unsigned char>());
      }

      static std::vector<unsigned char>&& event(const char* outcome) {
        return std::move(std::vector<unsigned char>());
      }

      static std::vector<unsigned char>&& report_action_taken() {
        return std::move(std::vector<unsigned char>());
      }
    };
  }
}
