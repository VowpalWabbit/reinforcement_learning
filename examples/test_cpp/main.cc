#include <boost/program_options.hpp>
#include <iostream>

#include "options.h"
#include "test_loop.h"

int run_test_instance(size_t index,
                      const boost::program_options::variables_map& vm) {
  test_loop loop(index, vm);

  if (!loop.init()) {
    std::cerr << "Test loop haven't initialized properly." << std::endl;
    return -1;
  }

  loop.run();

  return 0;
}

int main(int argc, char** argv) {
  try {
    const auto vm = process_cmd_line(argc, argv);
    if (is_help(vm)) return 0;

    const size_t num_instances = vm["instances"].as<size_t>();
    std::vector<std::thread> instances;
    for (size_t i = 0; i < num_instances; ++i) {
      instances.push_back(std::thread(&run_test_instance, i, vm));
    }
    for (size_t i = 0; i < num_instances; ++i) {
      instances[i].join();
    }
  }
  catch (const std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
    return -1;
  }
}
