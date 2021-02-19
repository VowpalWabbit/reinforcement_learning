#include "joined_log_parser.h"
#include "generated/v2/FileFormat_generated.h"

JoinedLogParser::JoinedLogParser(const std::string &initial_command_line)
    : example_joiner(initial_command_line) {}

int JoinedLogParser::read_header(const std::vector<char> &payload) {

  // TODO pass header info into the ExampleJoiner object

  auto file_header = v2::GetFileHeader(payload.data());
  std::cout << "header properties:" << std::endl;
  for (size_t i = 0; i < file_header->properties()->size(); i++) {
    std::cout << file_header->properties()->Get(i)->key()->c_str() << ":"
              << file_header->properties()->Get(i)->value()->c_str()
              << std::endl;
  }
  return err::success;
}

int JoinedLogParser::read_message(const std::vector<char> &payload) {
  auto joined_payload = flatbuffers::GetRoot<v2::JoinedPayload>(payload.data());
  for (size_t i = 0; i < joined_payload->events()->size(); i++) {
    // process events one-by-one
    example_joiner.process_event(*joined_payload->events()->Get(i));
  }
  example_joiner.train_on_joined();
  return err::success;
}
