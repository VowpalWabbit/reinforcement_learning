#pragma once

#include "hashstring.h"
#include <boost/version.hpp>

#if BOOST_VERSION < 106100
#include <boost/utility/string_ref.hpp>
namespace reinforcement_learning
{
using string_view = boost::string_ref;
}
#else
#include <boost/utility/string_view.hpp>
namespace reinforcement_learning
{
using string_view = boost::string_view;
}
#endif
