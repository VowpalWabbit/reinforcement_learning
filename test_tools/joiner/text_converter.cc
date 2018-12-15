#include <string>
#include <vector>
#include <fstream>
#include <flatbuffers/flatbuffers.h>
#include <iostream>
#include "../../rlclientlib/logger/preamble.h"
#include "../../rlclientlib/logger/message_type.h"
#include "../../rlclientlib/generated/RankingEvent_generated.h"
#include "../../rlclientlib/generated/OutcomeEvent_generated.h"

// namespace aliases
namespace rlog = reinforcement_learning::logger;
namespace flat = reinforcement_learning::messages::flatbuff;
////

namespace reinforcement_learning { namespace joiner {

  // forward declarations 
  void convert_to_text(const std::string& file);
  void print_ranking_event(void* buff);
  void print_outcome_event(void* buff);
  void print_numeric_outcome(const flat::OutcomeEventHolder* evt);
  void print_string_outcome(const flat::OutcomeEventHolder* evt);
  void print_action_outcome(const flat::OutcomeEventHolder* evt);
  ////

  void convert_to_text(const std::vector<std::string>& files) {
    for(const auto& file:files) {
      convert_to_text(file);
    }
  }

  void convert_to_text(const std::string& file){
    std::ifstream infile;
    infile.open(file, std::ios_base::binary);
    if (infile.fail() || infile.bad()){
      std::cout << "Unable to open file: " << file << std::endl;
    }
    std::cout << "File:" << file << std::endl;
    do {
      char raw_preamble[8];
      if (infile.fail() || infile.bad())
        return;
      infile.read(raw_preamble, 8);
      rlog::preamble p;
      p.read_from_bytes(reinterpret_cast<uint8_t*>(raw_preamble), 8);
      std::unique_ptr<char> msg_data(new char[p.msg_size]);
      infile.read(msg_data.get(), p.msg_size);
      if (infile.fail() || infile.bad())
        return;
      
      switch (p.msg_type) {
      case rlog::message_type::fb_ranking_event_collection:
        print_ranking_event(msg_data.get());
        break;
      case rlog::message_type::fb_outcome_event_collection:
        print_outcome_event(msg_data.get());
        break;
      default:
        break;
      }
    } while (!infile.fail() && !infile.bad());
  }

  inline std::string to_str(const flatbuffers::String* pstr) {
    return std::string(pstr->begin(), pstr->end());
  }

  inline std::string to_str(const flatbuffers::Vector<uint8_t>* pstr) {
    return std::string(pstr->begin(), pstr->end());
  }

  void print_ranking_event(void* buff)
  {
    const auto rank = flat::GetRankingEventBatch(buff);
    const auto events = rank->events();
    std::cout << "RankingBatch: ";
    for (auto evt : *events) {
      std::cout << "Int: ";

      std::cout << "id [" << to_str(evt->event_id()) << "]";

      std::cout << ", a [ ";
      for (auto i : *evt->action_ids()) {
        std::cout << i << ' ';
      }
      std::cout << "]";

      std::cout << ", p [ ";
      for (auto i : *evt->probabilities()) {
        std::cout << i << ' ';
      }
      std::cout << "]";

      std::cout << ", c [";
      std::cout << to_str(evt->context());
      std::cout << "]";

      std::cout << ", m [";
      std::cout << to_str(evt->model_id());
      std::cout << "]";

      std::cout << ", pass [" << evt->pass_probability() << "]";
      std::cout << ", def [" << evt->deferred_action() << "]" << std::endl;
    }
  }

  void print_outcome_event(void* buff) {
    const auto outcome = flat::GetOutcomeEventBatch(buff);
    const auto events = outcome->events();
    std::cout << "OutcomeBatch: ";
    for (auto evt : *events)
    {
      std::cout << "id [" << to_str(evt->event_id())  << "]";
      switch (evt->the_event_type()) {
      case flat::OutcomeEvent::OutcomeEvent_NumericEvent:
        print_numeric_outcome(evt);
        break;
      case flat::OutcomeEvent::OutcomeEvent_StringEvent:
        print_string_outcome(evt);
        break;
      case flat::OutcomeEvent::OutcomeEvent_ActionTakenEvent:
        print_action_outcome(evt);
        break;
      default:
        std::cout << ", outcome [UNKNOWN]";
      }
      std::cout << ", pass_prob [" << evt->pass_probability() << "]" << std::endl;
    }
  }

  void print_numeric_outcome(const flat::OutcomeEventHolder* evt) {
    const auto number = evt->the_event_as_NumericEvent();
    std::cout << ", outcome [" << number->value() << "]";
  }

  void print_string_outcome(const flat::OutcomeEventHolder* evt) {
    const auto str = evt->the_event_as_StringEvent();
    std::cout << ", outcome ['" << str->value() << "']";
  }

  void print_action_outcome(const flat::OutcomeEventHolder* evt) {
    const auto act = evt->the_event_as_ActionTakenEvent();
    std::cout << ", outcome [action_taken=" << act->value() << "]";
  }

}}