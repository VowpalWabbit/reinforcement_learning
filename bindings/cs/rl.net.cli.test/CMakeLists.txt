set (RL_NET_CLI_TEST_SOURCES
  CleanupContainer.cs
  LoopUsageTest.cs
  MockSender.cs
  ReplayStepProviderTest.cs
  SenderExtensibilityTest.cs
  SenderExtensibilityCBLoopTest.cs
  TempFileDisposable.cs
  TestBase.cs
  UnicodeTest.cs
)

if (rlclientlib_DOTNET_USE_MSPROJECT)
  include_external_msproject(rl.net.cli.test ${CMAKE_CURRENT_SOURCE_DIR}/rl.net.cli.test.csproj rl.net.cli)
else()
  # No need to add the other two targets to ALL, because the rl.net.cli.test target will chain-build them
  add_custom_target(rl.net.cli.test ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} --framework ${RL_DOTNET_TARGET_FRAMEWORK} -o ${RL_DOTNET_OUTPUT_DIRECTORY} -v m --nologo --no-dependencies /clp:NoSummary --configuration "$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:rlnetnative> ${RL_DOTNET_OUTPUT_DIRECTORY}
    COMMENT Building rl.net.cli.test
    DEPENDS rl.net.cli
    SOURCES ${RL_NET_CLI_TEST_SOURCES})
endif()

# TODO: Enable TRX test logging
add_test(
  NAME rl.net.cli.test
  COMMAND
    ${DOTNET_COMMAND} test ${RL_DOTNET_OUTPUT_DIRECTORY}/Rl.Net.Cli.Test.dll --Platform:x64 --InIsolation "--logger:console;verbosity=default"
)
set_tests_properties(rl.net.cli.test PROPERTIES WORKING_DIRECTORY $<TARGET_FILE_DIR:rlnetnative>)
if(WIN32)
  string(REPLACE ";" "\\;" RL_NET_TEST_PATH "$<TARGET_FILE_DIR:rlnetnative>;$ENV{PATH}")
  set_tests_properties(rl.net.cli.test PROPERTIES ENVIRONMENT "PATH=${RL_NET_TEST_PATH}")
elseif(APPLE)
  set_tests_properties(rl.net.cli.test PROPERTIES
    ENVIRONMENT "DYLD_LIBRARY_PATH=$<TARGET_FILE_DIR:rlnetnative>:$ENV{DYLD_LIBRARY_PATH}")
else()
  set_tests_properties(rl.net.cli.test PROPERTIES
    ENVIRONMENT "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:rlnetnative>:$ENV{LD_LIBRARY_PATH}")
endif()