#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <string>
#include <vector>

#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"

#include "vw/core/v_array.h"
#include "vw/core/vw.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;
constexpr float FLOAT_TOL = 0.0001f;

// learn/predict isn't called in the unit test but cleanup examples
// expects shared pred to be set for slates
void set_slates_label(v_array<example *> &examples);

void clear_examples(v_array<example *> &examples, VW::workspace *vw);

void set_buffer_as_vw_input(const std::vector<char> &buffer, VW::workspace *vw);

std::vector<char> read_file(std::string file_name);

std::string get_test_files_location();

std::vector<const v2::JoinedEvent *>
wrap_into_joined_events(std::vector<char> &buffer,
                        std::vector<flatbuffers::DetachedBuffer> &detached_buffers);
