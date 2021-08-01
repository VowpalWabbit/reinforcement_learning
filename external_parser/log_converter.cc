#include "log_converter.h"
#include "date.h"
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <rapidjson/ostreamwrapper.h>

namespace log_converter {
namespace rj = rapidjson;

template<typename OutputHandler>
struct CopyFilter {
    CopyFilter(OutputHandler& out) : _out(out) {
    }
 
    bool Null() { return _out.Null(); }
    bool Bool(bool b) { return _out.Bool(b); }
    bool Int(int i) { return _out.Int(i); }
    bool Uint(unsigned u) { return _out.Uint(u); }
    bool Int64(int64_t i) { return _out.Int64(i); }
    bool Uint64(uint64_t u) { return _out.Uint64(u); }
    bool Double(double d) { return _out.Double(d); }
    bool RawNumber(const char* str, rj::SizeType length, bool copy) { return _out.RawNumber(str, length, copy); }
    bool String(const char* str, rj::SizeType length, bool copy) { return _out.String(str, length, copy); }
    bool StartObject() { return _out.StartObject(); }
    bool Key(const char* str, rj::SizeType length, bool copy) {  return _out.Key(str, length, copy); }
    bool EndObject(rj::SizeType memberCount) { return _out.EndObject(memberCount); }
    bool StartArray() { return _out.StartArray(); }
    bool EndArray(rj::SizeType elementCount) { return _out.EndArray(elementCount); }
 
    OutputHandler& _out;
};
 
void build_cb_json(std::ofstream &outfile,
                   const joined_event::joined_event &je) {
  float cost = -1.f * reinterpret_cast<const joined_event::cb_joined_event *>(
                          je.get_hold_of_typed_data())
                          ->reward;
  float original_cost =
      -1.f * reinterpret_cast<const joined_event::cb_joined_event *>(
                 je.get_hold_of_typed_data())
                 ->original_reward;
  const auto &interaction_data =
      reinterpret_cast<const joined_event::cb_joined_event *>(
          je.get_hold_of_typed_data())
          ->interaction_data;
  const auto &probabilities = interaction_data.probabilities;
  const auto &actions = interaction_data.actions;
  try {
    rj::StringBuffer out_buffer;
    rj::Writer<rj::StringBuffer> writer(out_buffer);

    // rj::OStreamWrapper osw(outfile);
    // rj::Writer<rj::OStreamWrapper> writer(osw);

    int memberCount = 0;
    writer.StartObject();

    writer.Key("_label_cost", strlen("_label_cost"), true);
    writer.Double(cost);
    ++memberCount;

    float label_p =
        probabilities.size() > 0
            ? probabilities[0] * je.interaction_metadata.pass_probability
            : 0.f;
    writer.Key("_label_probability", strlen("_label_probability"), true);
    writer.Double(label_p);
    ++memberCount;

    writer.Key("_label_probability", strlen("_label_probability"), true);
    writer.Double(label_p);
    ++memberCount;

    writer.Key("_label_Action", strlen("_label_Action"), true);
    writer.Uint(actions[0]);
    ++memberCount;

    writer.Key("_labelIndex", strlen("_labelIndex"), true);
    writer.Uint(actions[0] - 1);
    ++memberCount;

    bool skip_learn = !je.is_joined_event_learnable();
    if (skip_learn) {
      writer.Key("_skipLearn", strlen("_skipLearn"), true);
      writer.Bool(skip_learn);
      ++memberCount;
    }

    writer.Key("o", strlen("o"), true);

    writer.StartArray();
    for (auto &o : je.outcome_events) {
      writer.StartObject();
      int ocount = 0;
      if (!o.action_taken) {
        writer.Key("v", strlen("v"), true);
        writer.Double(o.value);
        ++ocount;
      }
      writer.Key("EventId", strlen("EventId"), true);
      writer.String(o.metadata.event_id.c_str(), o.metadata.event_id.length(), true);
      ++ocount;

      writer.Key("ActionTaken", strlen("ActionTaken"), true);
      writer.Bool(o.action_taken);
      ++ocount;

      writer.EndObject(ocount);
    }

    writer.EndArray(je.outcome_events.size());
    ++memberCount;

    std::string ts_str = date::format(
        "%FT%TZ",
        date::floor<std::chrono::microseconds>(je.joined_event_timestamp));

    writer.Key("Timestamp", strlen("Timestamp"), true);
    writer.String(ts_str.c_str(), ts_str.length(), true);
    ++memberCount;

    writer.Key("Version", strlen("Version"), true);
    writer.String("1", strlen("1"), true);
    ++memberCount;

    writer.Key("EventId", strlen("EventId"), true);
    writer.String(interaction_data.eventId.c_str(), interaction_data.eventId.length(), true);
    ++memberCount;

    writer.Key("a", strlen("a"), true);
    writer.StartArray();

    for (auto &action_id : actions) {
      writer.Uint(action_id);
    }

    writer.EndArray(actions.size());
    ++memberCount;

    writer.Key("c");
    // FIXME: while this is significantly faster, we need to edit the context string and 
    // strip all line endinds as DSJSON requires one line per document
    writer.RawValue(je.context.c_str(), je.context.length(), rj::kObjectType);

    // CopyFilter<rj::Writer<rj::StringBuffer> > filter(writer);
    // rj::StringStream is(je.context.c_str());
    // rj::  Reader reader;
    // if (!reader.Parse<rj::kParseNumbersAsStringsFlag>(is, filter)) {
    //   throw new std::exception(); //fail, fixme
    // }
    ++memberCount;

    writer.Key("p", strlen("p"), true);
    writer.StartArray();
    for (auto &p : probabilities) {
      writer.Double(p);
    }

    writer.EndArray(probabilities.size());
    ++memberCount;

    writer.Key("VWState", strlen("VWState"), true);
    writer.StartObject();
    writer.Key("m", strlen("m"), true);
    writer.String(je.model_id.c_str(), je.model_id.length(), true);
    writer.EndObject(1);
    ++memberCount;

    writer.Key("_original_label_cost", strlen("_original_label_cost"), true);
    writer.Double(original_cost);
    ++memberCount;

    writer.EndObject(memberCount);

    outfile << out_buffer.GetString() << std::endl;
    // outfile.write("\n", 1); //don't use "<< std::endl" at that triggers a fflush
  } catch (const std::exception &e) {
    VW::io::logger::log_error(
        "convert events: [{}] from binary to json format failed: [{}].",
        interaction_data.eventId, e.what());
  }
}
} // namespace log_converter
