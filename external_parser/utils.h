// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/memory.h"
#include "vw/core/vw.h"

namespace VW
{
namespace external
{
bool stricmp(const char* first, const char* second);

template <typename enum_t>
bool str_to_enum(
    const std::string& str, const std::map<const char*, enum_t> enum_map, const enum_t default_value, enum_t& result)
{
  for (auto p : enum_map)
  {
    if (!stricmp(p.first, str.c_str()))
    {
      result = p.second;
      return true;
    }
  }
  result = default_value;
  return false;
}

}  // namespace external
}  // namespace VW
