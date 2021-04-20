#include "log_converter.h"

namespace log_converter {
void build_cb_json(std::ofstream &outfile, const joined_event &je,
                   float reward, float original_reward) {
  namespace rj = rapidjson;

  float cost = -1.f * reward;
  float original_cost = -1.f * original_reward;
  std::vector<float> probabilities = je.interaction_data.probabilities;
  std::vector<unsigned> actions = je.interaction_data.actions;

  try {
    rj::Document d;
    d.SetObject();
    rj::Document::AllocatorType& allocator = d.GetAllocator();

    d.AddMember("_label_cost", cost, allocator);

    float label_p = probabilities.size() > 0
      ? probabilities[0] * je.interaction_metadata.pass_probability
      : 0.f;
    d.AddMember("_label_probability", label_p, allocator);

    d.AddMember("_label_Action", actions[0], allocator);

    d.AddMember("_labelIndex", actions[0] - 1, allocator);

    rj::Value v;
    rj::Value outcome_arr(rj::kArrayType);
    for (auto &o : je.outcome_events) {
      rj::Value outcome(rj::kObjectType);
      outcome.AddMember("v", o.value, allocator);

      v.SetString(o.metadata.event_id.c_str(), allocator);
      outcome.AddMember("EventId", v, allocator);

      outcome.AddMember("ActionTaken", o.action_taken, allocator);
      outcome_arr.PushBack(outcome, allocator);
    }
    d.AddMember("o", outcome_arr, allocator);

    //TODO: add timestamp
    v.SetString(je.joined_event_timestamp.c_str(), allocator);
    d.AddMember("Timestamp", v, allocator);

    d.AddMember("Version", "1", allocator);

    v.SetString(je.interaction_data.eventId.c_str(), allocator);
    d.AddMember("EventId", v, allocator);

    rj::Value action_arr(rj::kArrayType);
    for (auto &action_id : actions) {
      action_arr.PushBack(action_id, allocator);
    }
    d.AddMember("a", action_arr, allocator);

    rj::Document context;
    context.Parse(je.context);
    d.AddMember("c", context, allocator);

    rj::Value p_arr(rj::kArrayType);
    for (auto &p : probabilities) {
      p_arr.PushBack(p, allocator);
    }
    d.AddMember("p", p_arr, allocator);

    rj::Value vwState;
    vwState.SetObject();
    {
      v.SetString(je.model_id.c_str(), allocator);
      vwState.AddMember("m", v, allocator);
    }
    d.AddMember("VWState", vwState, allocator);

    d.AddMember("_original_label_cost", original_cost, allocator);

    rj::StringBuffer sb;
    rj::Writer<rj::StringBuffer> writer(sb);
    d.Accept(writer);

    outfile << sb.GetString() << std::endl;
  }
  catch (const std::exception& e) {
    VW::io::logger::log_error(
      "convert events: [{}] from binary to json format failed: [{}].",
      je.interaction_data.eventId, e.what()
    );
  }
}
}

