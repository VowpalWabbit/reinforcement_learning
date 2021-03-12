#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <string>
#include <vector>

#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"

#include "v_array.h"
#include "vw.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

void clear_examples(v_array<example *> &examples, vw *vw);

void set_buffer_as_vw_input(const std::vector<char> &buffer, vw *vw);

std::vector<char> read_file(std::string file_name);

std::string get_test_files_location();

const v2::JoinedEvent *
wrap_into_joined_event(std::vector<char> &buffer,
                       flatbuffers::DetachedBuffer &detached_buffer);