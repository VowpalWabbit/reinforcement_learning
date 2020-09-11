#pragma once
#include <vector>
#include <flatbuffers/flatbuffers.h>
#include "logger/flatbuffer_allocator.h"
#include "generated/v1/OutcomeEvent_generated.h"
#include "generated/v1/RankingEvent_generated.h"
#include "generated/v1/DecisionRankingEvent_generated.h"
#include "generated/v1/SlatesEvent_generated.h"

#include "generated/v2/Event_generated.h"

#include "content_encoding.h"
#include "generic_event.h"
#include "ranking_event.h"

#include "logger/message_type.h"
#include "err_constants.h"
#include <iostream>

using namespace reinforcement_learning::messages::flatbuff;
namespace reinforcement_learning { namespace logger {
  template <typename T>
  struct fb_event_serializer;
  template <>
  struct fb_event_serializer<ranking_event> {
    using fb_event_t = RankingEvent;
    using offset_vector_t = typename std::vector<flatbuffers::Offset<fb_event_t>>;
    using batch_builder_t = RankingEventBatchBuilder;

    static size_t size_estimate(const ranking_event& evt) {
      return evt.get_event_id().size() + evt.get_action_ids().size() * sizeof(evt.get_action_ids()[0])
            + evt.get_probabilities().size() * sizeof(evt.get_probabilities()[0]) + evt.get_context().size()
            + evt.get_model_id().size() + sizeof(evt.get_defered_action()) + sizeof(evt.get_pass_prob()) + sizeof(evt.get_client_time_gmt());
    }

    static int serialize(ranking_event& evt, flatbuffers::FlatBufferBuilder& builder,
                         flatbuffers::Offset<fb_event_t>& ret_val, api_status* status) {
      const auto event_id_offset = builder.CreateString(evt.get_event_id());
      const auto action_ids_vector_offset = builder.CreateVector(evt.get_action_ids());
      const auto probabilities_vector_offset = builder.CreateVector(evt.get_probabilities());
      const auto context_offset = builder.CreateVector(evt.get_context());
      const auto model_id_offset = builder.CreateString(evt.get_model_id());
	    const auto &ts = evt.get_client_time_gmt();
      TimeStamp client_ts(	ts.year, ts.month, ts.day, ts.hour,
							ts.minute, ts.second, ts.sub_second);
	    const auto meta_id_offset = CreateMetadata(builder,&client_ts);

      LearningModeType learning_mode_type;
      switch (evt.get_learning_mode()) {
      case APPRENTICE: {
        learning_mode_type = LearningModeType_Apprentice;
        break;
      }
      case ONLINE: {
        learning_mode_type = LearningModeType_Online;
        break;
      }
      case LOGGINGONLY: {
        learning_mode_type = LearningModeType_LoggingOnly;
        break;
      }
      default:
        // This is to be back-compatible with the config not setting learning mode.
        break;
      }

      ret_val = CreateRankingEvent(	builder, event_id_offset, evt.get_defered_action(), action_ids_vector_offset,
									context_offset, probabilities_vector_offset, model_id_offset,
									evt.get_pass_prob(), meta_id_offset, learning_mode_type);
      return error_code::success;
    }
  };

  template <>
  struct fb_event_serializer<decision_ranking_event> {
    using fb_event_t = DecisionEvent;
    using offset_vector_t = typename std::vector<flatbuffers::Offset<fb_event_t>>;
    using batch_builder_t = DecisionEventBatchBuilder;

    static size_t size_estimate(const decision_ranking_event& evt) {
      size_t estimate = 0;
      auto action_ids = evt.get_actions_ids();
      auto probs = evt.get_probabilities();
      auto evt_ids = evt.get_event_ids();

      for (size_t i = 0; i < evt_ids.size(); i++)
      {
        estimate += action_ids[i].size() * sizeof(action_ids[0][0]);
        estimate += probs[i].size() * sizeof(probs[0][0]);
        estimate += evt_ids[i].size();
      }
      estimate += evt.get_context().size() + evt.get_model_id().size() + sizeof(evt.get_defered_action()) + sizeof(evt.get_pass_prob());
      estimate += sizeof(evt.get_client_time_gmt());
      return estimate;
    }

    static int serialize(decision_ranking_event& evt, flatbuffers::FlatBufferBuilder& builder,
                         flatbuffers::Offset<fb_event_t>& ret_val, api_status* status) {
      const auto context_offset = builder.CreateVector(evt.get_context());
      const auto model_id_offset = builder.CreateString(evt.get_model_id());

      auto action_ids = evt.get_actions_ids();
      const auto probabilities = evt.get_probabilities();
      const auto decision_slot_ids = evt.get_event_ids();
      std::vector<flatbuffers::Offset<SlotEvent>> slots;
      for (size_t i = 0; i < decision_slot_ids.size(); i++)
      {
        slots.push_back(CreateSlotEvent(builder, builder.CreateString(decision_slot_ids[i]), builder.CreateVector(action_ids[i]), builder.CreateVector(probabilities[i])));
      }
      const auto slots_offset = builder.CreateVector(slots);

      const auto &ts = evt.get_client_time_gmt();
      TimeStamp client_ts(ts.year, ts.month, ts.day, ts.hour,
        ts.minute, ts.second, ts.sub_second);
      const auto meta_id_offset = CreateMetadata(builder, &client_ts);

      ret_val = CreateDecisionEvent(builder, context_offset, slots_offset, model_id_offset, evt.get_pass_prob(), evt.get_defered_action(), meta_id_offset);
      return error_code::success;
    }
  };

  template <>
  struct fb_event_serializer<multi_slot_decision_event> {
    using fb_event_t = SlatesEvent;
    using offset_vector_t = typename std::vector<flatbuffers::Offset<fb_event_t>>;
    using batch_builder_t = SlatesEventBatchBuilder;

    static size_t size_estimate(const multi_slot_decision_event& evt) {
      size_t estimate = 0;
      auto action_ids = evt.get_actions_ids();
      auto probs = evt.get_probabilities();

      for (size_t i = 0; i < action_ids.size(); i++)
      {
        estimate += action_ids[i].size() * sizeof(action_ids[0][0]);
        estimate += probs[i].size() * sizeof(probs[0][0]);
      }

      estimate += evt.get_context().size() + evt.get_model_id().size() + sizeof(evt.get_defered_action()) + sizeof(evt.get_pass_prob());
      estimate += evt.get_event_id().size();
      estimate += sizeof(evt.get_client_time_gmt());
      return estimate;
    }

    static int serialize(multi_slot_decision_event& evt, flatbuffers::FlatBufferBuilder& builder,
                         flatbuffers::Offset<fb_event_t>& ret_val, api_status* status) {
      const auto event_id_offset = builder.CreateString(evt.get_event_id());
      const auto context_offset = builder.CreateVector(evt.get_context());
      const auto model_id_offset = builder.CreateString(evt.get_model_id());

      auto action_ids = evt.get_actions_ids();
      const auto probabilities = evt.get_probabilities();
      std::vector<flatbuffers::Offset<SlatesSlotEvent>> slots;
      for (size_t i = 0; i < action_ids.size(); i++)
      {
        slots.push_back(CreateSlatesSlotEvent(builder, builder.CreateVector(action_ids[i]), builder.CreateVector(probabilities[i])));
      }
      const auto slots_offset = builder.CreateVector(slots);

      const auto &ts = evt.get_client_time_gmt();
      TimeStamp client_ts(ts.year, ts.month, ts.day, ts.hour,
        ts.minute, ts.second, ts.sub_second);
      const auto meta_id_offset = CreateMetadata(builder, &client_ts);

      ret_val = CreateSlatesEvent(builder, event_id_offset, context_offset, slots_offset, model_id_offset, evt.get_pass_prob(), evt.get_defered_action(), meta_id_offset);
      return error_code::success;
    }
  };

  template <>
  struct fb_event_serializer<outcome_event> {
    using fb_event_t = OutcomeEventHolder;
    using offset_vector_t = std::vector<flatbuffers::Offset<fb_event_t>>;
    using batch_builder_t = OutcomeEventBatchBuilder;

    static size_t size_estimate(const outcome_event& evt) {
      return evt.get_event_id().size() + evt.get_outcome().size() + sizeof(evt.get_numeric_outcome()) + sizeof(evt.get_client_time_gmt());
    }

    static int serialize(outcome_event& evt, flatbuffers::FlatBufferBuilder& builder,
                         flatbuffers::Offset<fb_event_t>& retval, api_status* status) {
      const auto event_id = builder.CreateString(evt.get_event_id());
	    const auto &ts = evt.get_client_time_gmt();
	    TimeStamp client_ts(ts.year, ts.month, ts.day, ts.hour,
		  ts.minute, ts.second, ts.sub_second);
	    const auto meta_id_offset = CreateMetadata(builder, &client_ts);
	    switch (evt.get_outcome_type()) {
        case outcome_event::outcome_type_string: {
          const auto outcome_str = builder.CreateString(evt.get_outcome());
          const auto str_event = CreateStringEvent(builder, outcome_str).Union();
          retval = CreateOutcomeEventHolder(builder, event_id, evt.get_pass_prob(), OutcomeEvent_StringEvent,
                                            str_event, meta_id_offset);
          break;
        }
        case outcome_event::outcome_type_numeric: {
          const auto number_event = CreateNumericEvent(builder, evt.get_numeric_outcome()).Union();
          retval = CreateOutcomeEventHolder(builder, event_id, evt.get_pass_prob(), OutcomeEvent_NumericEvent,
                                            number_event, meta_id_offset);
          break;
        }
        case outcome_event::outcome_type_action_taken: {
          const auto action_taken_event = CreateActionTakenEvent(builder, evt.get_action_taken()).Union();
          retval = CreateOutcomeEventHolder(builder, event_id, evt.get_pass_prob(), OutcomeEvent_ActionTakenEvent,
                                            action_taken_event, meta_id_offset);
          break;
        }
        default: {
          RETURN_ERROR(nullptr, status, serialize_unknown_outcome_type);
        }
      }
      return error_code::success;
    }
  };

  template <typename event_t>
  struct fb_collection_serializer {
    using serializer_t = fb_event_serializer<event_t>;
    using buffer_t = utility::data_buffer;
    static int message_id() { return message_type::UNKNOWN; }

    fb_collection_serializer(buffer_t& buffer, content_encoding_enum content_encoding = content_encoding_enum::IDENTITY)
      : _allocator(buffer), _builder(buffer.body_capacity(), &_allocator), _buffer(buffer), _content_encoding(content_encoding)  {}

    int add(event_t& evt, api_status* status = nullptr) {
      flatbuffers::Offset<typename serializer_t::fb_event_t> offset;
      RETURN_IF_FAIL(serializer_t::serialize(evt, _builder, offset, status));
      _event_offsets.push_back(offset);
      return error_code::success;
    }

    uint64_t size() const { return _builder.GetSize(); }

    void create_header() {
      return;
    }

    void add_header(typename serializer_t::batch_builder_t &batch_builder)
    {
      return;
    }

    void finalize() {
      auto event_offsets = _builder.CreateVector(_event_offsets);
      create_header();
      typename serializer_t::batch_builder_t batch_builder(_builder);
      batch_builder.add_events(event_offsets);
      add_header(batch_builder);
      auto batch_offset = batch_builder.Finish();
      _builder.Finish(batch_offset);
      // Where does the body of the data begin in relation to the start
      // of the raw buffer
      const auto offset = _builder.GetBufferPointer() - _buffer.raw_begin();
      _buffer.set_body_endoffset(_buffer.preamble_size() + _buffer.body_capacity());
      _buffer.set_body_beginoffset(offset);
    }

    typename serializer_t::offset_vector_t _event_offsets;
    flatbuffer_allocator _allocator;
    flatbuffers::FlatBufferBuilder _builder;
    buffer_t& _buffer;
    content_encoding_enum _content_encoding;
    flatbuffers::Offset<reinforcement_learning::messages::flatbuff::v2::BatchMetadata> _batch_metadata_offset;
  };

  template <>
  struct fb_event_serializer<generic_event> {
    using fb_event_t = v2::Event;
    using offset_vector_t = typename std::vector<flatbuffers::Offset<fb_event_t>>;
    using batch_builder_t = v2::EventBatchBuilder;

    static size_t size_estimate(const generic_event& evt) {
      return evt.get_payload().size();
    }

    static int serialize(generic_event& evt, flatbuffers::FlatBufferBuilder& builder,
      flatbuffers::Offset<fb_event_t>& ret_val, api_status* status) {

      const auto id_offset = builder.CreateString(evt.get_id());

      const auto& ts = evt.get_client_time_gmt();
      v2::TimeStamp client_ts(ts.year, ts.month, ts.day, ts.hour,
        ts.minute, ts.second, ts.sub_second);
      const auto meta_offset = v2::CreateMetadataDirect(builder, evt.get_id(), &client_ts, nullptr, evt.get_payload_type(), evt.get_pass_prob());
      const auto& buffer = evt.get_payload();
      const auto payload_offset = builder.CreateVector(buffer.data(), buffer.size());
      ret_val = v2::CreateEvent(builder, meta_offset, payload_offset);
      return error_code::success;
    }
  };

  template <>
  inline int fb_collection_serializer<outcome_event>::message_id() { return message_type::fb_outcome_event_collection; }

  template <>
  inline int fb_collection_serializer<decision_ranking_event>::message_id() { return message_type::fb_decision_event_collection; }

  template <>
  inline int fb_collection_serializer<multi_slot_decision_event>::message_id() { return message_type::fb_slates_event_collection; }

  template <>
  inline int fb_collection_serializer<ranking_event>::message_id() { return message_type::fb_ranking_learning_mode_event_collection; }

  template <>
  inline void fb_collection_serializer<generic_event>::create_header()
  {
    const auto batch_metadata_offset = v2::CreateBatchMetadataDirect(_builder, to_content_encoding_string(_content_encoding));
    _batch_metadata_offset = batch_metadata_offset;
    return;
  }

  template <>
  inline void fb_collection_serializer<generic_event>::add_header(typename serializer_t::batch_builder_t& batch_builder)
  {
    batch_builder.add_metadata(_batch_metadata_offset);
  }
}}
