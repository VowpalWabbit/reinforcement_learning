// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/vw_exception.h"
#include "vw/config/option_group_definition.h"
#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/global_data.h"
#include "vw/core/input_parser.h"
#include "vw/core/memory.h"
#include "vw/core/metric_sink.h"
#include "vw/core/vw.h"

#include "parse_example_external.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>

using namespace VW::config;

int main(int argc, char *argv[]) {
  bool should_use_onethread = false;
  std::string log_level;
  std::string log_output_stream;
  option_group_definition driver_config("Driver");
  driver_config.add(make_option("onethread", should_use_onethread)
                        .help("Disable parse thread"));
  driver_config.add(make_option("log_level", log_level)
                        .default_value("info")
                        .hidden()
                        .one_of({"info", "warn", "error", "critical", "off"})
                        .help("Log level for logging messages. Specifying this "
                              "wil override --quiet for log output"));
  driver_config.add(make_option("log_output", log_output_stream)
                        .default_value("stdout")
                        .hidden()
                        .one_of({"stdout", "stderr", "compat"})
                        .help("Specify the stream to output log messages to. "
                              "In the past VW's choice of stream for "
                              "logging messages wasn't consistent. Supplying "
                              "compat will maintain that old behavior. "
                              "Compat is now deprecated so it is recommended "
                              "that stdout or stderr is chosen"));

  try {
    auto options = VW::make_unique<options_cli>(
        std::vector<std::string>(argv + 1, argv + argc));
    options->add_and_parse(driver_config);
    auto all = VW::external::initialize_with_binary_parser(std::move(options));
    all->vw_is_main = true;

    auto skip_driver = all->options->get_typed_option<bool>("dry_run").value();
    if (skip_driver) {
      // Leave deletion up to the unique_ptr
      VW::finish(*all, false);
      return 0;
    }

    if (should_use_onethread) {
      VW::LEARNER::generic_driver_onethread(*all);
    } else {
      VW::start_parser(*all);
      VW::LEARNER::generic_driver(*all);
      VW::end_parser(*all);
    }

    if (all->example_parser->exc_ptr) {
      std::rethrow_exception(all->example_parser->exc_ptr);
    }

    VW::sync_stats(*all);
    // Leave deletion up to the unique_ptr
    VW::finish(*all, false);
  } catch (const VW::vw_exception &e) {
    if (log_level != "off") {
      if (log_output_stream == "compat" || log_output_stream == "stderr") {
        std::cerr << "[critical] vw (" << e.Filename() << ":" << e.LineNumber()
                  << "): " << e.what() << std::endl;
      } else {
        std::cout << "[critical] vw (" << e.Filename() << ":" << e.LineNumber()
                  << "): " << e.what() << std::endl;
      }
    }
    return 1;
  } catch (const std::exception &e) {
    // vw is implemented as a library, so we use 'throw runtime_error()'
    // error 'handling' everywhere.  To reduce stderr pollution
    // everything gets caught here & the error message is printed
    // sans the excess exception noise, and core dump.
    // TODO: If loggers are instantiated within struct vw, this line lives
    // outside of that. Log as critical for now
    if (log_level != "off") {
      if (log_output_stream == "compat" || log_output_stream == "stderr") {
        std::cerr << "[critical] vw: " << e.what() << std::endl;
      } else {
        std::cout << "[critical] vw: " << e.what() << std::endl;
      }
    }
    return 1;
  } catch (...) {
    if (log_level != "off") {
      if (log_output_stream == "compat" || log_output_stream == "stderr") {
        std::cerr << "[critical] Unknown exception occurred" << std::endl;
      } else {
        std::cout << "[critical] vw: unknown exception" << std::endl;
      }
    }
    return 1;
  }

  return 0;
}
