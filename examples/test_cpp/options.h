#pragma once

#include <boost/program_options.hpp>
#include <string>

boost::program_options::variables_map process_cmd_line(const int argc, char** argv);
bool is_help(const boost::program_options::variables_map& vm);
void throw_if_conflicting(
    const boost::program_options::variables_map& vm, const std::string& first, const std::string& second);
