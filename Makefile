CXX ?= $(shell which g++)
# -- if you want to test 32-bit use this instead,
#    it sometimes reveals type portability issues
# CXX = $(shell which g++) -m32
ifneq ($(CXX),)
  #$(warning Using clang: "$(CXX)")
  ARCH = -D__extern_always_inline=inline
else
  CXX = clang++
  $(warning Using clang++)
endif
#ARCH = $(shell test `$CXX -v 2>&1 | tail -1 | cut -d ' ' -f 3 | cut -d '.' -f 1,2` \< 4.3 && echo -march=nocona || echo -march=native)

ifeq ($(CXX),)
  $(warning No compiler found)
  exit 1
endif

UNAME := $(shell uname)
ARCH_UNAME := $(shell uname -m)
LIBS = -l boost_program_options -l pthread -l z
BOOST_INCLUDE = -I /usr/local/include/boost -I /usr/include
BOOST_LIBRARY = -L /usr/local/lib -L /usr/lib
NPROCS := 1

ifeq ($(UNAME), Linux)
  BOOST_LIBRARY += -L /usr/lib/x86_64-linux-gnu
  NPROCS:=$(shell grep -c ^processor /proc/cpuinfo)
endif
ifeq ($(UNAME), FreeBSD)
  LIBS = -l boost_program_options -l pthread -l z -l compat
  BOOST_INCLUDE = -I /usr/local/include
  NPROCS:=$(shell grep -c ^processor /proc/cpuinfo)
endif
ifeq "CYGWIN" "$(findstring CYGWIN,$(UNAME))"
  LIBS = -l boost_program_options-mt -l pthread -l z
  BOOST_INCLUDE = -I /usr/include
  NPROCS:=$(shell grep -c ^processor /proc/cpuinfo)
endif
ifeq ($(UNAME), Darwin)
  LIBS = -lboost_program_options-mt -lboost_serialization-mt -l pthread -l z
  # On Macs, the location isn't always clear
  #	brew uses /usr/local
  #	but /opt/local seems to be preferred by some users
  #	so we try them both
  ifneq (,$(wildcard /usr/local/include))
    BOOST_INCLUDE = -I /usr/local/include
    BOOST_LIBRARY = -L /usr/local/lib
  endif
  ifneq (,$(wildcard /opt/local/include))
    BOOST_INCLUDE = -I /opt/local/include
    BOOST_LIBRARY = -L /opt/local/lib
  endif
  NPROCS:=$(shell sysctl -n hw.ncpu)
endif

ifneq ($(USER_BOOST_INCLUDE),)
  BOOST_INCLUDE = $(USER_BOOST_INCLUDE)
endif
ifneq ($(USER_BOOST_LIBRARY),)
  BOOST_LIBRARY = $(USER_BOOST_LIBRARY)
endif

JSON_INCLUDE = -I ext_libs/vowpal_wabbit/rapidjson/include

#LIBS = -l boost_program_options-gcc34 -l pthread -l z

ifeq ($(ARCH_UNAME), ppc64le)
  OPTIM_FLAGS ?= -DNDEBUG -O3 -fomit-frame-pointer -fno-strict-aliasing #-msse2 is not supported on power
else
  OPTIM_FLAGS ?= -DNDEBUG -O3 -fomit-frame-pointer -fno-strict-aliasing -msse2 -mfpmath=sse #-ffast-math #uncomment for speed, comment for testability
endif

ifeq ($(UNAME), FreeBSD)
  WARN_FLAGS = -Wall
else
  WARN_FLAGS = -Wall -pedantic
endif

# for normal fast execution.
FLAGS = -std=c++11 $(CFLAGS) $(LDFLAGS) $(ARCH) $(WARN_FLAGS) $(OPTIM_FLAGS) -D_FILE_OFFSET_BITS=64 $(BOOST_INCLUDE) $(JSON_INCLUDE) -fPIC #-DVW_LDA_NO_SSE
FLAGS += -I ext_libs/vowpal_wabbit/explore

default:	vw

all:	vw rl_clientlib

export

rl_clientlib: vw
	cd rlclientlib; $(MAKE) -j $(NPROCS) things

# Devirtualization is an optimization that changes the vtable if the compiler decides a function
# doesn't need to be virtual. This is incompatible with the mocking framework used in testing as it
# makes the vtable structure unpredictable
rl_clientlib_test: FLAGS += -fno-devirtualize
rl_clientlib_test: vw rl_clientlib
	cd unit_test; $(MAKE) -j $(NPROCS) things
	(cd unit_test && ./rlclient-test.out)

rl_example: vw rl_clientlib
	cd examples/basic_usage_cpp; $(MAKE) -j $(NPROCS) things
	cd examples/rl_sim_cpp; $(MAKE) -j $(NPROCS) things
	cd examples/test_cpp; $(MAKE) -j $(NPROCS) things
	cd examples/override_interface; $(MAKE) -j $(NPROCS) things

rl_python: vw rl_clientlib
	cd bindings/python; $(MAKE) -j $(NPROCS) things

vw:
	cd ext_libs/vowpal_wabbit; $(MAKE) -j $(NPROCS) vw

clean:
	cd ext_libs/vowpal_wabbit && $(MAKE) clean
	cd rlclientlib && $(MAKE) clean
	cd unit_test; $(MAKE) clean
	cd bindings/python; $(MAKE) clean
	cd examples/basic_usage_cpp; $(MAKE) clean
	cd examples/rl_sim_cpp; $(MAKE) clean
	cd examples/test_cpp; $(MAKE) clean

.PHONY: all clean install
