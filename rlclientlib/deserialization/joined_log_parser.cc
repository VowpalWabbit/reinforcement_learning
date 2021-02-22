#include <cstring>
#include <fstream>
#include <iostream>

#include "err_constants.h"
#include "example_joiner.h"
#include "joined_log_parser.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;
namespace err = reinforcement_learning::error_code;

JoinedLogParser::JoinedLogParser(const std::string &initial_command_line)
    : example_joiner(VW::make_unique<ExampleJoiner>(initial_command_line)) {}

JoinedLogParser::~JoinedLogParser() = default;

// TODO make better error messages
int JoinedLogParser::read_and_deserialize_file(const std::string &file_name) {
  std::ifstream fs(file_name.c_str(), std::ifstream::binary);
  std::vector<char> buffer(4, 0);
  const std::vector<char> magic = {'V', 'W', 'F', 'B'};

  // read the 4 magic bytes
  fs.read(buffer.data(), buffer.size());
  if (buffer != magic) {
    return err::file_read_error;
  }

  // read the version
  fs.read(buffer.data(), buffer.size());
  if (static_cast<int>(buffer[0]) != 1) {
    return err::file_read_error;
  }

  // payload type, check for header
  unsigned int payload_type;
  fs.read((char *)(&payload_type), sizeof(payload_type));
  if (payload_type != MSG_TYPE_HEADER) {
    return err::file_read_error;
  }

  // read header size
  fs.read(buffer.data(), buffer.size());
  uint32_t payload_size = *reinterpret_cast<const uint32_t *>(buffer.data());
  // read the payload
  std::vector<char> payload(payload_size, 0);
  fs.read(payload.data(), payload.size());

  read_header(payload);

  fs.read((char *)(&payload_type), sizeof(payload_type));
  while (payload_type != MSG_TYPE_EOF) {
    if (payload_type != MSG_TYPE_REGULAR)
      return err::file_read_error;
    // read payload size
    fs.read(buffer.data(), buffer.size());
    uint32_t payload_size = *reinterpret_cast<const uint32_t *>(buffer.data());
    // read the payload
    std::vector<char> payload(payload_size, 0);
    fs.read(payload.data(), payload.size());
    if (read_message(payload) != err::success)
      return err::file_read_error;
    fs.read((char *)(&payload_type), sizeof(payload_type));
  }

  return err::success;
}

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
    example_joiner->process_event(*joined_payload->events()->Get(i));
  }
  example_joiner->train_on_joined();
  return err::success;
}
