set (RL_NET_CLI_TEST_SOURCES
  CleanupContainer.cs
  ReplayStepProviderTest.cs
  SenderExtensibilityTest.cs
  TempFileDisposable.cs
  TestBase.cs
  UnicodeTest.cs
)

# No need to add the other two targets to ALL, because the rl.net.cli.test target will chain-build them
add_custom_target(rl.net.cli.test ALL
  COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:rl.net.native> -v m --nologo --no-dependencies /clp:NoSummary
  COMMENT Building rl.net.cli.test
  DEPENDS rl.net.cli
  SOURCES ${RL_NET_CLI_TEST_SOURCES})

# TODO: Enable TRX test logging
add_test(
  NAME rl.net.cli.test
  COMMAND
    ${DOTNET_COMMAND} test $<TARGET_FILE_DIR:rl.net.native>/rl.net.cli.test.dll --Platform:x64 --InIsolation "--logger:console;verbosity=default"
)