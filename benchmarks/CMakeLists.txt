set(all_sources
  benchmark_cb_v2.cc
  benchmark_ccb.cc
  benchmark_common.cc
  benchmark_init.cc
  benchmark_main.cc
)

add_executable(rl_benchmarks
  ${all_sources}
)

find_package(benchmark REQUIRED)

# Add the include directories from rlclientlib target for testing
target_include_directories(rl_benchmarks PRIVATE $<TARGET_PROPERTY:rlclientlib,INCLUDE_DIRECTORIES>)
target_link_libraries(rl_benchmarks PRIVATE rlclientlib benchmark::benchmark)

# Communicate that Boost Unit Test is being statically linked
if(RL_STATIC_DEPS)
  target_compile_definitions(rl_benchmarks PRIVATE RL_STATIC_DEPS)
endif()

add_test(rl_benchmarks rl_benchmarks)