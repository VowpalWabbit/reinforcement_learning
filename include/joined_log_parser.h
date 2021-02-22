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
  // reads and processes the entire file
  int read_and_deserialize_file(const std::string &file_name);
  // reads process the header payload
  int read_header(const std::vector<char> &payload);
  // reads and process regular message payloads
  int read_message(const std::vector<char> &payload);

private:
  std::unique_ptr<ExampleJoiner> example_joiner;
};