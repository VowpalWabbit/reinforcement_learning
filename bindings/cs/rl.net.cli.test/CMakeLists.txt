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
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:rlnetnative> -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT Building rl.net.cli.test
    DEPENDS rl.net.cli rl.net.flatbuffers
    SOURCES ${RL_NET_CLI_TEST_SOURCES})
endif()

# TODO: Enable TRX test logging
add_test(
  NAME rl.net.cli.test
  COMMAND
    ${DOTNET_COMMAND} test $<TARGET_FILE_DIR:rlnetnative>/rl.net.cli.test.dll --Platform:x64 --InIsolation "--logger:console;verbosity=default"
)