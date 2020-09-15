#pragma once

#pragma once

#include <vector>

#include <flatbuffers/flatbuffers.h>

#include "action_flags.h"
#include "ranking_event.h"
#include "generic_event.h"
#include "data_buffer.h"
#include "logger/message_type.h"
#include "api_status.h"
#include "utility/data_buffer_streambuf.h"
#include "learning_mode.h"

#include "generated/v2/OutcomeSingle_generated.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/MultiSlotEvent_generated.h"

namespace reinforcement_learning {
  namespace logger {
    using namespace messages::flatbuff;

    int get_learning_mode(learning_mode mode_in, v2::LearningModeType& mode_out, api_status* status);

    template<generic_event::payload_type_t pt>
    struct payload_serializer {
      const generic_event::payload_type_t type = pt;
    };

    struct cb_serializer : payload_serializer<generic_event::payload_type_t::PayloadType_CB> {
      static generic_event::payload_buffer_t event(const char* context, unsigned int flags, v2::LearningModeType learning_mode, const ranking_response& response) {
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

        auto fb = v2::CreateCbEventDirect(fbb, flags & action_flags::DEFERRED, &action_ids, &_context, &probabilities, response.get_model_id(), learning_mode);
        fbb.Finish(fb);
        return fbb.Release();
      }
    };

    struct multi_slot_serializer : payload_serializer<generic_event::payload_type_t::PayloadType_MultiSlot> {
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

        auto fb = v2::CreateMultiSlotEventDirect(fbb, &_context, &slots, model_version.c_str(), flags & action_flags::DEFERRED);
        fbb.Finish(fb);
        return fbb.Release();
      }
    };

    struct outcome_single_serializer : payload_serializer<generic_event::payload_type_t::PayloadType_OutcomeSingle> {
      static generic_event::payload_buffer_t numeric_event(float outcome) {
        flatbuffers::FlatBufferBuilder fbb;
        const auto evt = v2::CreateNumericOutcome(fbb, outcome).Union();
        auto fb = v2::CreateOutcomeSingleEvent(fbb, v2::OutcomeValue_numeric, evt);
        fbb.Finish(fb);
        return fbb.Release();
      }

      static generic_event::payload_buffer_t string_event(const char* outcome) {
        flatbuffers::FlatBufferBuilder fbb;
        const auto evt = fbb.CreateString(outcome).Union();
        auto fb = v2::CreateOutcomeSingleEvent(fbb, v2::OutcomeValue_literal, evt);
        fbb.Finish(fb);
        return fbb.Release();
      }

      static generic_event::payload_buffer_t numeric_event(int index, float outcome) {
        flatbuffers::FlatBufferBuilder fbb;
        const auto evt = v2::CreateNumericOutcome(fbb, outcome).Union();
        auto fb = v2::CreateOutcomeSingleEvent(fbb, v2::OutcomeValue_numeric, evt, false, index);
        fbb.Finish(fb);
        return fbb.Release();
      }

      static generic_event::payload_buffer_t string_event(int index, const char* outcome) {
        flatbuffers::FlatBufferBuilder fbb;
        const auto evt = fbb.CreateString(outcome).Union();
        auto fb = v2::CreateOutcomeSingleEvent(fbb, v2::OutcomeValue_literal, evt, false, index);
        fbb.Finish(fb);
        return fbb.Release();
      }

      static generic_event::payload_buffer_t report_action_taken() {
        flatbuffers::FlatBufferBuilder fbb;
        auto fb = v2::CreateOutcomeSingleEvent(fbb, v2::OutcomeValue_NONE, 0, true);
        fbb.Finish(fb);
        return fbb.Release();
      }
    };
  }
}
