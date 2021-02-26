#include "joined_log_parser.h"
#include <boost/program_options.hpp>
#include <cstring>
#include <iostream>
#include <string>

namespace po = boost::program_options;

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

  // TODO get file name and vw command from command line
  JoinedLogParser parser("--cb_explore_adf -f modelfile.model --invert_hash inv_hash.inv -p p.txt");
  return parser.read_and_deserialize_file(file_name);
}