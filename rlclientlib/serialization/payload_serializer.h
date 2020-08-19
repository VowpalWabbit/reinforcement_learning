#pragma once

#pragma once

#include <vector>

#include <flatbuffers/flatbuffers.h>

#include "action_flags.h"
#include "ranking_event.h"
#include "data_buffer.h"
#include "logger/message_type.h"
#include "api_status.h"
#include "utility/data_buffer_streambuf.h"
#include "learning_mode.h"

#include "generated/v2/OutcomeSingle_generated.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/CcbEvent_generated.h"
#include "generated/v2/SlatesEvent_generated.h"

namespace reinforcement_learning {
  namespace logger {
    using namespace messages::flatbuff;

    v2::LearningModeType GetLearningMode(learning_mode mode);

    template<payload_type pt>
    struct payload_serializer {
      const payload_type type = pt;
    };

    struct cb_serializer : payload_serializer<payload_type::CB> {
      static generic_event::payload_buffer_t event(const char* context, unsigned int flags, learning_mode learning_mode, const ranking_response& response) {
        flatbuffers::FlatBufferBuilder fbb;
        std::vector<uint64_t> action_ids;
        std::vector<float> probabilities;
        for (auto const& r : response) {
          action_ids.push_back(r.action_id + 1);
          probabilities.push_back(r.probability);
        }
        std::vector<unsigned char> _context;
        std::string context_str(context);
        copy(context_str.begin(), context_str.end(), std::back_inserter(_context));

        auto fb = v2::CreateCbEventDirect(fbb, flags | action_flags::DEFERRED, &action_ids, &_context, &probabilities, response.get_model_id(), GetLearningMode(learning_mode));
        fbb.Finish(fb);
        return fbb.Release();
      }
    };

    struct ccb_serializer : payload_serializer<payload_type::CCB> {
      static generic_event::payload_buffer_t event(const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
        const std::vector<std::vector<float>>& pdfs, const std::string& model_version) {
        flatbuffers::FlatBufferBuilder fbb;
        std::vector<flatbuffers::Offset<v2::SlotEvent>> slots;
        for (size_t i = 0; i < action_ids.size(); i++)
        {
          slots.push_back(v2::CreateSlotEventDirect(fbb, &action_ids[i], &pdfs[i]));
        }

        std::vector<unsigned char> _context;
        std::string context_str(context);
        copy(context_str.begin(), context_str.end(), std::back_inserter(_context));

        auto fb = v2::CreateCcbEventDirect(fbb, &_context, &slots, model_version.c_str(), flags | action_flags::DEFERRED);
        fbb.Finish(fb);
        return fbb.Release();
      }
    };

    struct slates_serializer : payload_serializer<payload_type::SLATES> {
      static generic_event::payload_buffer_t event(const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
        const std::vector<std::vector<float>>& pdfs, const std::string& model_version) {
        flatbuffers::FlatBufferBuilder fbb;
        std::vector<flatbuffers::Offset<v2::SlatesSlotEvent>> slots;
        for (size_t i = 0; i < action_ids.size(); i++)
        {
          slots.push_back(v2::CreateSlatesSlotEventDirect(fbb, &action_ids[i], &pdfs[i]));
        }

        std::vector<unsigned char> _context;
        std::string context_str(context);
        copy(context_str.begin(), context_str.end(), std::back_inserter(_context));

        auto fb = v2::CreateSlatesEventDirect(fbb, &_context, &slots, model_version.c_str(), flags | action_flags::DEFERRED);
        fbb.Finish(fb);
        return fbb.Release();
      }
    };

    struct outcome_single_serializer : payload_serializer<payload_type::OUTCOME_SINGLE> {
      static generic_event::payload_buffer_t event(float outcome) {
        flatbuffers::FlatBufferBuilder fbb;
        const auto evt = v2::CreateNumericEventSingle(fbb, outcome).Union();
        auto fb = v2::CreateOutcomeSingleEvent(fbb, v2::OutcomeSingleEventBody_NumericEventSingle, evt);
        fbb.Finish(fb);
        return fbb.Release();
      }

      static generic_event::payload_buffer_t event(const char* outcome) {
        flatbuffers::FlatBufferBuilder fbb;
        const auto evt = v2::CreateStringEventSingleDirect(fbb, outcome).Union();
        auto fb = v2::CreateOutcomeSingleEvent(fbb, v2::OutcomeSingleEventBody_StringEventSingle, evt);
        fbb.Finish(fb);
        return fbb.Release();
      }

      static generic_event::payload_buffer_t event(int index, float outcome) {
        flatbuffers::FlatBufferBuilder fbb;
        const auto evt = v2::CreateNumericEventIndexed(fbb, outcome, index).Union();
        auto fb = v2::CreateOutcomeSingleEvent(fbb, v2::OutcomeSingleEventBody_NumericEventIndexed, evt);
        fbb.Finish(fb);
        return fbb.Release();
      }

      static generic_event::payload_buffer_t event(int index, const char* outcome) {
        flatbuffers::FlatBufferBuilder fbb;
        const auto evt = v2::CreateStringEventIndexedDirect(fbb, outcome, index).Union();
        auto fb = v2::CreateOutcomeSingleEvent(fbb, v2::OutcomeSingleEventBody_StringEventIndexed, evt);
        fbb.Finish(fb);
        return fbb.Release();
      }

      static generic_event::payload_buffer_t&& report_action_taken() {
        flatbuffers::FlatBufferBuilder fbb;
        const auto evt = v2::CreateActionTakenEvent(fbb, true).Union();
        auto fb = v2::CreateOutcomeSingleEvent(fbb, v2::OutcomeSingleEventBody_ActionTakenEvent, evt);
        fbb.Finish(fb);
        return fbb.Release();
      }
    };
  }
}
