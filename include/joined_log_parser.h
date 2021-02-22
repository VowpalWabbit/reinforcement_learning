#pragma once

#include <string>
#include <vector>
#include <memory>

const unsigned int MSG_TYPE_HEADER = 0x55555555;
const unsigned int MSG_TYPE_REGULAR = 0xFFFFFFFF;
const unsigned int MSG_TYPE_EOF = 0xAAAAAAAA;

class ExampleJoiner;

class JoinedLogParser {
public:
  explicit JoinedLogParser(
      const std::string &initial_command_line); // TODO Rule of 5
  ~JoinedLogParser();
  // reads and process the entire file
  int read_and_deserialize_file(const std::string &file_name);
  // reads the header and adds it to the example joiner
  int read_header(const std::vector<char> &payload);
  // reads all the events from the joined payload and passes them to
  int read_message(const std::vector<char> &payload);

private:
  std::unique_ptr<ExampleJoiner> example_joiner;
};