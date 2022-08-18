#pragma once

#pragma once

#include "action_flags.h"
#include "api_status.h"
#include "continuous_action_response.h"
#include "data_buffer.h"
#include "generated/v2/CaEvent_generated.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/DedupInfo_generated.h"
#include "generated/v2/MultiSlotEvent_generated.h"
#include "generated/v2/MultiStepEvent_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"
#include "generic_event.h"
#include "learning_mode.h"
#include "logger/message_type.h"
#include "ranking_event.h"
#include "rl_string_view.h"
#include "utility/data_buffer_streambuf.h"

#include <flatbuffers/flatbuffers.h>

#include <vector>

namespace reinforcement_learning
{
namespace logger
{
using namespace messages::flatbuff;

int get_learning_mode(learning_mode mode_in, v2::LearningModeType& mode_out, api_status* status);

template <generic_event::payload_type_t pt>
struct payload_serializer
{
  const generic_event::payload_type_t type = pt;
};

struct cb_serializer : payload_serializer<generic_event::payload_type_t::PayloadType_CB>
{
  static generic_event::payload_buffer_t event(const char* context, unsigned int flags,
      v2::LearningModeType learning_mode, const std::vector<uint64_t>& action_ids,
      const std::vector<float>& probabilities, const std::string& model_id)
  {
    flatbuffers::FlatBufferBuilder fbb;

    std::vector<unsigned char> _context;
    std::string context_str(context);
    copy(context_str.begin(), context_str.end(), std::back_inserter(_context));

    auto fb = v2::CreateCbEventDirect(
        fbb, flags & action_flags::DEFERRED, &action_ids, &_context, &probabilities, model_id.c_str(), learning_mode);
    fbb.Finish(fb);
    return fbb.Release();
  }
};

struct ca_serializer : payload_serializer<generic_event::payload_type_t::PayloadType_CA>
{
  static generic_event::payload_buffer_t event(const char* context, unsigned int flags, float chosen_action,
      float chosen_action_pdf_value, const std::string& model_id)
  {
    flatbuffers::FlatBufferBuilder fbb;

    std::vector<unsigned char> _context;
    std::string context_str(context);
    copy(context_str.begin(), context_str.end(), std::back_inserter(_context));

    auto fb = v2::CreateCaEventDirect(
        fbb, flags & action_flags::DEFERRED, chosen_action, &_context, chosen_action_pdf_value, model_id.c_str());
    fbb.Finish(fb);
    return fbb.Release();
  }
};

struct multi_slot_serializer : payload_serializer<generic_event::payload_type_t::PayloadType_Slates>
{
  static generic_event::payload_buffer_t event(const char* context, unsigned int flags,
      const std::vector<std::vector<uint32_t>>& action_ids, const std::vector<std::vector<float>>& pdfs,
      const std::string& model_version, const std::vector<std::string>& slot_ids,
      const std::vector<int>& baseline_actions, v2::LearningModeType learning_mode)
  {
    flatbuffers::FlatBufferBuilder fbb;
    std::vector<flatbuffers::Offset<v2::SlotEvent>> slots;

    std::vector<unsigned char> _context;
    std::string context_str(context);
    copy(context_str.begin(), context_str.end(), std::back_inserter(_context));

    for (size_t i = 0; i < action_ids.size(); i++)
    { slots.push_back(v2::CreateSlotEventDirect(fbb, &action_ids[i], &pdfs[i], slot_ids[i].c_str())); }
    auto fb = v2::CreateMultiSlotEventDirect(fbb, &_context, &slots, model_version.c_str(),
        flags & action_flags::DEFERRED, &baseline_actions, learning_mode);
    fbb.Finish(fb);
    return fbb.Release();
  }
};

struct dedup_info_serializer : payload_serializer<generic_event::payload_type_t::PayloadType_DedupInfo>
{
  static generic_event::payload_buffer_t event(
      const std::vector<generic_event::object_id_t>& object_ids, const std::vector<string_view>& object_values)
  {
    flatbuffers::FlatBufferBuilder fbb;
    std::vector<flatbuffers::Offset<flatbuffers::String>> vals;
    vals.reserve(object_values.size());

    for (auto sv : object_values) { vals.push_back(fbb.CreateString(sv.begin(), sv.size())); }

    auto fb = v2::CreateDedupInfoDirect(fbb, &object_ids, &vals);
    fbb.Finish(fb);
    return fbb.Release();
  }
};

struct outcome_serializer : payload_serializer<generic_event::payload_type_t::PayloadType_Outcome>
{
  static generic_event::payload_buffer_t numeric_event(float outcome)
  {
    flatbuffers::FlatBufferBuilder fbb;
    const auto evt = v2::CreateNumericOutcome(fbb, outcome).Union();
    auto fb = v2::CreateOutcomeEvent(fbb, v2::OutcomeValue_numeric, evt);
    fbb.Finish(fb);
    return fbb.Release();
  }

  static generic_event::payload_buffer_t string_event(const char* outcome)
  {
    flatbuffers::FlatBufferBuilder fbb;
    const auto evt = fbb.CreateString(outcome).Union();
    auto fb = v2::CreateOutcomeEvent(fbb, v2::OutcomeValue_literal, evt);
    fbb.Finish(fb);
    return fbb.Release();
  }

  static generic_event::payload_buffer_t numeric_event(int index, float outcome)
  {
    flatbuffers::FlatBufferBuilder fbb;
    const auto evt = v2::CreateNumericOutcome(fbb, outcome).Union();
    const auto idx = v2::CreateNumericIndex(fbb, index).Union();
    auto fb = v2::CreateOutcomeEvent(fbb, v2::OutcomeValue_numeric, evt, v2::IndexValue_numeric, idx);
    fbb.Finish(fb);
    return fbb.Release();
  }

  static generic_event::payload_buffer_t numeric_event(const char* index, float outcome)
  {
    flatbuffers::FlatBufferBuilder fbb;
    const auto evt = v2::CreateNumericOutcome(fbb, outcome).Union();
    const auto idx = fbb.CreateString(index).Union();
    auto fb = v2::CreateOutcomeEvent(fbb, v2::OutcomeValue_numeric, evt, v2::IndexValue_literal, idx);
    fbb.Finish(fb);
    return fbb.Release();
  }

  static generic_event::payload_buffer_t string_event(int index, const char* outcome)
  {
    flatbuffers::FlatBufferBuilder fbb;
    const auto evt = fbb.CreateString(outcome).Union();
    const auto idx = v2::CreateNumericIndex(fbb, index).Union();
    auto fb = v2::CreateOutcomeEvent(fbb, v2::OutcomeValue_literal, evt, v2::IndexValue_numeric, idx);
    fbb.Finish(fb);
    return fbb.Release();
  }

  static generic_event::payload_buffer_t string_event(const char* index, const char* outcome)
  {
    flatbuffers::FlatBufferBuilder fbb;
    const auto evt = fbb.CreateString(outcome).Union();
    const auto idx = fbb.CreateString(index).Union();
    auto fb = v2::CreateOutcomeEvent(fbb, v2::OutcomeValue_literal, evt, v2::IndexValue_literal, idx);
    fbb.Finish(fb);
    return fbb.Release();
  }

  static generic_event::payload_buffer_t report_action_taken()
  {
    flatbuffers::FlatBufferBuilder fbb;
    auto fb = v2::CreateOutcomeEvent(fbb, v2::OutcomeValue_NONE, 0, v2::IndexValue_NONE, 0, true);
    fbb.Finish(fb);
    return fbb.Release();
  }

  static generic_event::payload_buffer_t report_action_taken(const char* index)
  {
    flatbuffers::FlatBufferBuilder fbb;
    const auto idx = fbb.CreateString(index).Union();
    auto fb = v2::CreateOutcomeEvent(fbb, v2::OutcomeValue_NONE, 0, v2::IndexValue_literal, idx, true);
    fbb.Finish(fb);
    return fbb.Release();
  }
};

struct multistep_serializer : payload_serializer<generic_event::payload_type_t::PayloadType_MultiStep>
{
  static generic_event::payload_buffer_t event(const char* context, const std::string& previous_id, unsigned int flags,
      const std::vector<uint64_t>& action_ids, const std::vector<float>& probabilities, const std::string& event_id,
      const std::string& model_id)
  {
    flatbuffers::FlatBufferBuilder fbb;

    std::vector<unsigned char> _context;
    std::string context_str(context);
    copy(context_str.begin(), context_str.end(), std::back_inserter(_context));

    auto fb = v2::CreateMultiStepEventDirect(fbb, event_id.c_str(), previous_id.c_str(), &action_ids, &_context,
        &probabilities, model_id.c_str(), flags & action_flags::DEFERRED);
    fbb.Finish(fb);
    return fbb.Release();
  }
};

struct episode_serializer : payload_serializer<generic_event::payload_type_t::PayloadType_Episode>
{
  static generic_event::payload_buffer_t episode_event(const char* event_id)
  {
    flatbuffers::FlatBufferBuilder fbb;
    auto fb = v2::CreateEpisodeEventDirect(fbb, event_id);
    fbb.Finish(fb);
    return fbb.Release();
  }
};
}  // namespace logger
}  // namespace reinforcement_learning
