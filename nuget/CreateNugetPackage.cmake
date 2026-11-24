find_program(
  nuget_exe
  NAMES "nuget.exe" "NuGet.exe"
  HINTS "${CMAKE_BINARY_DIR}/nuget/" "${CMAKE_BINARY_DIR}/../nuget/" "${CMAKE_BINARY_DIR}/../../nuget/" "${CMAKE_BINARY_DIR}/../../../nuget/"
  NO_CACHE
  REQUIRED
)

message("Creating Nuget package...")
message("Path to nuget.exe: ${nuget_exe}")
message("Working directory: ${CMAKE_INSTALL_PREFIX}")

execute_process(
  COMMAND "${nuget_exe}" pack rlclientlib.nuspec
  WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
  RESULT_VARIABLE return_code
)

if(return_code)
  message(FATAL_ERROR "Failed to build Nuget package: nuget.exe returned ${return_code}")
endif()