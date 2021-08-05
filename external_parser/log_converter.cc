#include "log_converter.h"
#include "date.h"
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <rapidjson/ostreamwrapper.h>

namespace log_converter {
namespace rj = rapidjson;
 
void build_cb_json(std::ofstream &outfile,
                   joined_event::joined_event &je) {
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

    writer.StartObject();

    writer.Key("_label_cost", strlen("_label_cost"), true);
    writer.Double(cost);

    float label_p =
        probabilities.size() > 0
            ? probabilities[0] * je.interaction_metadata.pass_probability
            : 0.f;
    writer.Key("_label_probability", strlen("_label_probability"), true);
    writer.Double(label_p);

    writer.Key("_label_Action", strlen("_label_Action"), true);
    writer.Uint(actions[0]);

    writer.Key("_labelIndex", strlen("_labelIndex"), true);
    writer.Uint(actions[0] - 1);

    bool skip_learn = !je.is_joined_event_learnable();
    if (skip_learn) {
      writer.Key("_skipLearn", strlen("_skipLearn"), true);
      writer.Bool(skip_learn);
    }

    writer.Key("o", strlen("o"), true);

    writer.StartArray();
    for (auto &o : je.outcome_events) {
      writer.StartObject();
      if (!o.action_taken) {
        writer.Key("v", strlen("v"), true);
        writer.Double(o.value);
      }
      writer.Key("EventId", strlen("EventId"), true);
      writer.String(o.metadata.event_id.c_str(), o.metadata.event_id.length(), true);

      writer.Key("ActionTaken", strlen("ActionTaken"), true);
      writer.Bool(o.action_taken);

      writer.EndObject();
    }

    writer.EndArray();

    std::string ts_str = date::format(
        "%FT%TZ",
        date::floor<std::chrono::microseconds>(je.joined_event_timestamp));

    writer.Key("Timestamp", strlen("Timestamp"), true);
    writer.String(ts_str.c_str(), ts_str.length(), true);

    writer.Key("Version", strlen("Version"), true);
    writer.String("1", strlen("1"), true);

    writer.Key("EventId", strlen("EventId"), true);
    writer.String(interaction_data.eventId.c_str(), interaction_data.eventId.length(), true);

    writer.Key("a", strlen("a"), true);
    writer.StartArray();

    for (auto &action_id : actions) {
      writer.Uint(action_id);
    }

    writer.EndArray();

    writer.Key("c");
    std::replace(je.context.begin(), je.context.end(), '\n', ' ');
    writer.RawValue(je.context.c_str(), je.context.length(), rj::kObjectType);

    writer.Key("p", strlen("p"), true);
    writer.StartArray();
    for (auto &p : probabilities) {
      writer.Double(p);
    }

    writer.EndArray();

    writer.Key("VWState", strlen("VWState"), true);
    writer.StartObject();
    writer.Key("m", strlen("m"), true);
    writer.String(je.model_id.c_str(), je.model_id.length(), true);
    writer.EndObject();

    writer.Key("_original_label_cost", strlen("_original_label_cost"), true);
    writer.Double(original_cost);

    writer.EndObject();

    outfile << out_buffer.GetString() << std::endl;
  } catch (const std::exception &e) {
    VW::io::logger::log_error(
        "convert events: [{}] from binary to json format failed: [{}].",
        interaction_data.eventId, e.what());
  }
}
} // namespace log_converter
