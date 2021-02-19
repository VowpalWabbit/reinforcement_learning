#pragma once

#include "err_constants.h"
#include "example_joiner.h" // TODO should this be here? is it ok?

namespace v2 = reinforcement_learning::messages::flatbuff::v2;
namespace err = reinforcement_learning::error_code;

const unsigned int MSG_TYPE_HEADER = 0x55555555;
const unsigned int MSG_TYPE_REGULAR = 0xFFFFFFFF;
const unsigned int MSG_TYPE_EOF = 0xAAAAAAAA;

class JoinedLogParser {
public:
  explicit JoinedLogParser(
      const std::string &initial_command_line); // TODO Rule of 5
  ~JoinedLogParser() = default;
  // reads the header and adds it to the example joiner
  int read_header(const std::vector<char> &payload);
  // reads all the events from the joined payload and passes them to
  int read_message(const std::vector<char> &payload);

private:
  ExampleJoiner example_joiner;
};
