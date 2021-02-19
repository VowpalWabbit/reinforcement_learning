#include <boost/program_options.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "joined_log_parser.h"
#include "example_joiner.h"

namespace po = boost::program_options;

// TODO: what assumptions should we make about endianness?
// TODO make better error messages
int read_and_deserialize_file(const std::string &file_name) {
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
  // data is in the payload now can use it to deserialize header
  JoinedLogParser logparser("--quiet --cb_explore_adf -f amodel.model");
  logparser.read_header(payload);

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
    if (logparser.read_message(payload) != err::success)
      return err::file_read_error;
    fs.read((char *)(&payload_type), sizeof(payload_type));
  }

  return err::success;
}

int main(int argc, char *argv[]) {
  po::options_description desc("binary file parser");
  std::string file_name;
  desc.add_options()("help", "Produce help message")(
      "file", po::value<std::string>(),
      "the file containing the joined logs in binary format");

  po::positional_options_description pd;
  pd.add("file", 1);

  po::variables_map vm;
  try {
    store(
        po::command_line_parser(argc, argv).options(desc).positional(pd).run(),
        vm);
    po::notify(vm);
    if (vm.count("file") > 0)
      file_name = vm["file"].as<std::string>();
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    std::cout << desc << std::endl;
    return 0;
  }

  if (vm.count("help") > 0 || file_name.empty()) {
    std::cout << desc << std::endl;
    return 0;
  }

  return read_and_deserialize_file(file_name);
}