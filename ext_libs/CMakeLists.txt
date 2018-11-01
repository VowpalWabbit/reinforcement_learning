add_subdirectory(fakeit)

set(VW_TEST OFF CACHE BOOL "Disable VW test target")
set(WARNINGS OFF CACHE BOOL "Disable VW warnings")
add_subdirectory(vowpal_wabbit EXCLUDE_FROM_ALL)
