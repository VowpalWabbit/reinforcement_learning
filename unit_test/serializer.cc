#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "logger/flatbuffer_allocator.h"
#include "ranking_event.h"
#include "utility/data_buffer.h"

using namespace reinforcement_learning;

using IBuffer = utility::data_buffer;

template <typename T>
struct FBSerial
{
};

template <>
struct FBSerial<ranking_event>
{
  using Type = RankingEvent;
  using OffsetVector = std::vector<flatbuffers::Offset<Type>>;
  using BatchBuilder = messages::RankingEventBatchBuilder;

  static void serialize(ranking_event& evt, OffsetVector& offsets, flatbuffers::FlatBufferBuilder& builder)
  {
    const auto event_id_offset = builder.CreateString(evt.get_event_id());
    const auto action_ids_vector_offset = builder.CreateVector(evt.get_action_ids());
    const auto probabilities_vector_offset = builder.CreateVector(evt.get_probabilities());
    const auto context_offset = builder.CreateVector(evt.get_context());
    const auto model_id_offset = builder.CreateString(evt.get_model_id());
    const auto offset = messages::CreateRankingEvent(builder, event_id_offset, evt.get_defered_action(),
        action_ids_vector_offset, context_offset, probabilities_vector_offset, model_id_offset);
    offsets.push_back(offset);
  }
};

template <typename Event>
struct FBCollectionSerializer
{
  using Serial = FBSerial<Event>;
  FBCollectionSerializer(IBuffer& buffer)
      : _builder{buffer.capacity(), &_allocator}, _allocator(buffer), _buffer(buffer)
  {
  }

  void add(Event& evt) { Serial::serialize(evt, _event_offsets, _builder); }

  uint64_t size() const { return _builder.GetSize(); }

  void finalize()
  {
    auto event_offsets = _builder.CreateVector(_event_offsets);
    typename Serial::BatchBuilder batch_builder(_builder);
    batch_builder.add_Events(event_offsets);
    auto orc = batch_builder.Finish();
    _builder.Finish(orc);
    _buffer.set_begin(_builder.GetBufferPointer(), _builder.GetSize());
  }

  typename Serial::OffsetVector _event_offsets;
  flatbuffers::FlatBufferBuilder _builder;
  flatbuffer_allocator _allocator;
  IBuffer& _buffer;
};

struct ISender
{
  void send(IBuffer&& buffer) {}
};

template <typename Event, template <typename> class Serializer>
void DrainQueue()
{
  IBuffer buffer;
  const uint64_t high_water_mark = 1024 * 100;
  Serializer<Event> serializer(buffer);
  ISender sender;
  Event rank;

  while (serializer.size() < high_water_mark) { serializer.add(rank); }
  serializer.finalize();
  sender.send(std::move(buffer));
}

BOOST_AUTO_TEST_CASE(new_data_buffer_is_empty)
{
  DrainQueue<ranking_event, FBCollectionSerializer>();
  //  DrainQueue<outcome_event, FBCollectionSerializer>(); // won't compile because specialization does not exist
}
