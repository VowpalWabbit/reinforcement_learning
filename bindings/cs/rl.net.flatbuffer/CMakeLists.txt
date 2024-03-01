set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../cmake/Modules/")

# First try to find the config version. Newer, used by vcpkg etc
find_package(Flatbuffers CONFIG)
if(TARGET flatbuffers::flatbuffers AND TARGET flatbuffers::flatc)
  get_property(flatc_location TARGET flatbuffers::flatc PROPERTY LOCATION)
  message(STATUS "Found Flatbuffers with CONFIG, flatc located at: ${flatc_location}")
else()
  # Fallback to the old version
  find_package(Flatbuffers MODULE REQUIRED)
  set(flatc_location ${FLATBUFFERS_FLATC_EXECUTABLE})
  message(STATUS "Found Flatbuffers with MODULE, flatc located at: ${flatc_location}")
endif()

include(FlatbufferUtils)

set(RL_NET_VWFB_SCHEMA_FILES
  "${CMAKE_SOURCE_DIR}/ext_libs/vowpal_wabbit/vowpalwabbit/fb_parser/schema/example.fbs" )

set(RL_NET_FLATBUFFERS_SOURCES
  VWExampleBuilder.cs
)

add_flatbuffer_schema(
  TARGET rl.net.flatbuffers_generated
  SCHEMAS ${RL_NET_VWFB_SCHEMA_FILES}
  OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated/vwfb
  GENERATED_FILES_VAR RL_NET_FLATBUFFERS_SOURCES
  FLATC_LANGUAGE CSharp
  FLATC_EXE ${flatc_location}
)

# find all the files

if (rlclientlib_DOTNET_USE_MSPROJECT)
  include_external_msproject(rl.net.flatbuffers ${CMAKE_CURRENT_SOURCE_DIR}/rl.net.flatbuffers.csproj)
else()
  add_custom_target(rl.net.flatbuffers
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:rlnetnative> -v n --nologo --no-dependencies /clp:NoSummary --configuration "$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>"
      COMMENT Building rl.net.flatbuffers
      SOURCES ${RL_NET_FLATBUFFERS_SOURCES})
endif()

add_dependencies(rl.net.flatbuffers rl.net.flatbuffers_generated)