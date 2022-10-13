#pragma once

#if __cplusplus >= 201103L || defined(_MSC_VER) && (_MSC_VER >= 1900)
#  if __cplusplus >= 201402L || defined(_MSC_VER) && (_MSC_VER >= 1910) && (_MSVC_LANG >= 201402L)
#    define HAS_STD14
#  endif
#  if __cplusplus >= 201703L || defined(_MSC_VER) && (_MSC_VER >= 1914) && (_MSVC_LANG >= 201703L)
#    define HAS_STD17
#  endif
#else
#  error "At least C++11 is required."
#endif

#ifdef HAS_STD17
#  define RL_ATTR(name) [[name]]
#else
#  define RL_ATTR(name)
#endif

#ifdef HAS_STD14
#  define RL_STD14_CONSTEXPR constexpr
#  define RL_DEPRECATED(message) [[deprecated(message)]]
#  if defined(__clang__)
#    define RL_IGNORE_DEPRECATED_USAGE_START \
      _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#    define RL_IGNORE_DEPRECATED_USAGE_END _Pragma("GCC diagnostic pop")
#  elif defined(__GNUC__) || defined(__GNUG__)
#    define RL_IGNORE_DEPRECATED_USAGE_START \
      _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#    define RL_IGNORE_DEPRECATED_USAGE_END _Pragma("GCC diagnostic pop")
#  elif defined(_MSC_VER)
#    define RL_IGNORE_DEPRECATED_USAGE_START __pragma(warning(disable : 4996))
#    define RL_IGNORE_DEPRECATED_USAGE_END __pragma(warning(default : 4996))
#  endif
#else
#  define RL_STD14_CONSTEXPR
#  define RL_DEPRECATED(message)
#  define RL_IGNORE_DEPRECATED_USAGE_START
#  define RL_IGNORE_DEPRECATED_USAGE_END
#endif
