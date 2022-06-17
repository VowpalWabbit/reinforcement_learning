#include "text_converter.h"

#include "../../rlclientlib/generated/v1/OutcomeEvent_generated.h"
#include "../../rlclientlib/generated/v1/RankingEvent_generated.h"
#include "../../rlclientlib/logger/message_type.h"
#include "../../rlclientlib/logger/preamble.h"

#include <flatbuffers/flatbuffers.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
// namespace aliases
namespace rlog = reinforcement_learning::logger;
namespace flat = reinforcement_learning::messages::flatbuff;
////

namespace reinforcement_learning
{
namespace joiner
{

// forward declarations
void convert_to_text(const std::string& file);
void convert_to_text(std::istream& in_strm, std::ostream& out_strm);
void print_ranking_event(void* buff, std::ostream& out_strm);
void print_outcome_event(void* buff, std::ostream& out_strm);
void print_numeric_outcome(const flat::OutcomeEventHolder* evt, std::ostream& out_strm);
void print_string_outcome(const flat::OutcomeEventHolder* evt, std::ostream& out_strm);
void print_action_outcome(const flat::OutcomeEventHolder* evt, std::ostream& out_strm);
////

void convert_to_text(const std::vector<std::string>& files)
{
  for (const auto& file : files) { convert_to_text(file); }
}

void convert_to_text(const std::string& file)
{
  std::ifstream infile;
  infile.open(file, std::ios_base::binary);
  if (infile.fail() || infile.bad()) { std::cout << "Unable to open file: " << file << std::endl; }
  std::cout << "File:" << file << std::endl;

  convert_to_text(infile, std::cout);
}

void convert_to_text(std::istream& in_strm, std::ostream& out_strm)
{
  do {
    if (in_strm.fail() || in_strm.bad())
    {
      std::cerr << "Error in input stream." << std::endl;
      return;
    }

    char raw_preamble[8];
    in_strm.read(raw_preamble, 8);
    rlog::preamble p;
    p.read_from_bytes(reinterpret_cast<uint8_t*>(raw_preamble), 8);
    std::unique_ptr<char> msg_data(new char[p.msg_size]);
    in_strm.read(msg_data.get(), p.msg_size);
    if (in_strm.fail() || in_strm.bad())
    {
      std::cerr << "Error reading from input file." << std::endl;
      return;
    }

    switch (p.msg_type)
    {
      case rlog::message_type::fb_ranking_learning_mode_event_collection:
        print_ranking_event(msg_data.get(), out_strm);
        break;
      case rlog::message_type::fb_outcome_event_collection:
        print_outcome_event(msg_data.get(), out_strm);
        break;
      default:
        break;
    }
  } while (!in_strm.fail() && !in_strm.bad());
}

inline std::string to_str(const flatbuffers::String* pstr) { return std::string(pstr->begin(), pstr->end()); }

inline std::string to_str(const flatbuffers::Vector<uint8_t>* pstr) { return std::string(pstr->begin(), pstr->end()); }

inline std::string to_str(const messages::flatbuff::Metadata* pmeta)
{
  // "04/11/19 hh:mm:ss.mmm.xxxx"
  std::ostringstream s;
  s << std::setfill('0') << std::setw(2);
  s << (int)pmeta->client_time_utc()->month() << "/";
  s << (int)pmeta->client_time_utc()->day() << "/";
  s << std::setw(4);
  s << pmeta->client_time_utc()->year() << " ";
  s << std::setw(2);
  s << (int)pmeta->client_time_utc()->hour() << ":";
  s << (int)pmeta->client_time_utc()->minute() << ":";
  s << (int)pmeta->client_time_utc()->second() << ".";
  const auto us = pmeta->client_time_utc()->subsecond() % 10000;
  const auto ms = (pmeta->client_time_utc()->subsecond() - us) / 10000;
  s << ms << ".";
  s << std::setw(4);
  s << us;
  return s.str();
}

void print_ranking_event(void* buff, std::ostream& out_strm)
{
  const auto rank = flat::GetRankingEventBatch(buff);
  const auto events = rank->events();
  out_strm << "RankingBatch: ";
  for (auto evt : *events)
  {
    out_strm << "Int: ";

    out_strm << "[" << to_str(evt->meta()) << "]";

    out_strm << "id [" << to_str(evt->event_id()) << "]";

    out_strm << ", a [ ";
    for (auto i : *evt->action_ids()) { out_strm << i << ' '; }
    out_strm << "]";

    out_strm << ", p [ ";
    for (auto i : *evt->probabilities()) { out_strm << i << ' '; }
    out_strm << "]";

    out_strm << ", c [";
    out_strm << to_str(evt->context());
    out_strm << "]";

    out_strm << ", m [";
    out_strm << to_str(evt->model_id());
    out_strm << "]";

    out_strm << ", pass [" << evt->pass_probability() << "]";
    out_strm << ", def [" << evt->deferred_action() << "]" << std::endl;
  }
}

void print_outcome_event(void* buff, std::ostream& out_strm)
{
  const auto outcome = flat::GetOutcomeEventBatch(buff);
  const auto events = outcome->events();
  out_strm << "OutcomeBatch: ";
  for (auto evt : *events)
  {
    out_strm << "[" << to_str(evt->meta()) << "] ";
    out_strm << "id [" << to_str(evt->event_id()) << "]";
    switch (evt->the_event_type())
    {
      case flat::OutcomeEvent::OutcomeEvent_NumericEvent:
        print_numeric_outcome(evt, out_strm);
        break;
      case flat::OutcomeEvent::OutcomeEvent_StringEvent:
        print_string_outcome(evt, out_strm);
        break;
      case flat::OutcomeEvent::OutcomeEvent_ActionTakenEvent:
        print_action_outcome(evt, out_strm);
        break;
      default:
        out_strm << ", outcome [UNKNOWN]";
    }
    out_strm << ", pass_prob [" << evt->pass_probability() << "]" << std::endl;
  }
}

void print_numeric_outcome(const flat::OutcomeEventHolder* evt, std::ostream& out_strm)
{
  const auto number = evt->the_event_as_NumericEvent();
  out_strm << ", outcome [" << number->value() << "]";
}

void print_string_outcome(const flat::OutcomeEventHolder* evt, std::ostream& out_strm)
{
  const auto str = evt->the_event_as_StringEvent();
  out_strm << ", outcome ['" << str->value() << "']";
}

void print_action_outcome(const flat::OutcomeEventHolder* evt, std::ostream& out_strm)
{
  const auto act = evt->the_event_as_ActionTakenEvent();
  out_strm << ", outcome [action_taken=" << act->value() << "]";
}

}  // namespace joiner
}  // namespace reinforcement_learning
