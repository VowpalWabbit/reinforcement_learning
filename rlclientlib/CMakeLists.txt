add_library(rlclientlib
  src/api_status.cc
  src/error_callback_fn.cc
  src/factory_resolver.cc
  src/live_model_impl.cc
  src/live_model.cc
  src/logger/async_batcher.cc
  src/logger/event_logger.cc
  src/logger/eventhub_client.cc
  src/model_mgmt/data_callback_fn.cc
  src/model_mgmt/model_downloader.cc
  src/model_mgmt/model_mgmt.cc
  src/model_mgmt/restapi_data_transport.cc
  src/ranking_event.cc
  src/ranking_response_impl.cc
  src/ranking_response.cc
  src/utility/config_utility.cc
  src/utility/configuration.cc
  src/utility/context_helper.cc
  src/utility/data_buffer.cc
  src/utility/str_util.cc
  src/utility/watchdog.cc
  src/vw_model/safe_vw.cc
  src/vw_model/vw_model.cc
)

add_subdirectory(unit_test)

target_include_directories(rlclientlib
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    src
    ${vw_inc_dirs}
    ${rapidjson_inc_dirs})

set_target_properties(rlclientlib PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# target_compile_features(rlclientlib PRIVATE cxx_std_11)
target_link_libraries(rlclientlib PRIVATE cpprestsdk::cpprest Boost::system vw)

# # 'make install' to the correct locations (provided by GNUInstallDirs).
# install(TARGETS rlclientlib EXPORT MyLibraryConfig
#     ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
# install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# # This makes the project importable from the install directory
# # Put config file in per-project dir (name MUST match), can also
# # just go into 'cmake'.
# install(EXPORT MyLibraryConfig DESTINATION share/MyLibrary/cmake)

# # This makes the project importable from the build directory
# export(TARGETS rlclientlib FILE MyLibraryConfig.cmake)

# Every library has unit tests, of course
# add_executable(testlib
#     test/testlib.cpp)

# target_link_libraries(testlib
#     rlclientlib)

# add_test(testlib testlib)
