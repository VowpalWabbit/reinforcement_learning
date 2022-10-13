// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <cstring>
#ifndef _WIN32
#  define _stricmp strcasecmp
#endif

namespace VW
{
namespace external
{
bool stricmp(const char* first, const char* second) { return _stricmp(first, second); }
}  // namespace external
}  // namespace VW