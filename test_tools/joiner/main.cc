// main.cc : This file contains the 'main' function. Program execution begins and ends there.
//

#include "text_converter.h"

#include <boost/program_options.hpp>
#include <iostream>

// namespace aliases
namespace po = boost::program_options;
namespace joiner = reinforcement_learning::joiner;
////

// Forward declarations
bool is_help(const po::variables_map& vm);
void parse_and_run(int argc, char** argv);
////

// Entry point
int main(const int argc, char** argv)
{
  try
  {
    parse_and_run(argc, argv);
  }
  catch (const std::exception& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
    return -1;
  }
}

bool is_help(const po::variables_map& vm) { return vm.count("help") > 0; }

// Parse command line args and run appropriate commands
void parse_and_run(int argc, char** argv)
{
  po::options_description desc("Options");
  desc.add_options()("help", "produce help message")("print,p", po::value<bool>()->default_value(false),
      "Print out contents of raw log files.  (interaction.fb.data, observation.fb.data)")("join,j",
      po::value<bool>()->default_value(false),
      "Join the interaction and observation files and create a file to be consumed by vw for training");

  po::variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);

  if (is_help(vm)) { std::cout << desc << std::endl; }
  else if (vm["print"].as<bool>())
  {
    joiner::convert_to_text({"interaction.fb.data", "observation.fb.data"});
  }
  else if (vm["join"].as<bool>())
  {
    std::cout << "Coming soon..." << std::endl;
  }
  else
  {
    std::cout << desc << std::endl;
  }
}