find_program(NUGET_COMMAND "nuget" REQUIRED
  HINTS ${CMAKE_CURRENT_LIST_DIR}/../../.nuget
        ENV nugetPath)

message(STATUS "Found nuget: ${NUGET_COMMAND}")