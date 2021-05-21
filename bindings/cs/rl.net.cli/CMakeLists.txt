set (RL_NET_CLI_SOURCES
  CommandBase.cs
  EntryPoints.cs
  Helpers.cs
  InternalsVisibleToTest.tt
  PerfTestCommand.cs
  PerfTestStepProvider.cs
  Person.cs
  ReplayCommand.cs
  ReplayStepProvider.cs
  RLDriver.cs
  RLSimulator.cs
  RobotJoint.cs
  RunSimulatorCommand.cs
  Statistics.cs
  StatisticsCalculator.cs
)

if (rlclientlib_DOTNET_USE_MSPROJECT)
  include_external_msproject(rl.net.cli ${CMAKE_CURRENT_SOURCE_DIR}/rl.net.cli.csproj rl.net)
else()
  add_custom_target(rl.net.cli
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:rl.net.native> -v m --nologo --no-dependencies /clp:NoSummary
    COMMENT Building rl.net.cli
    DEPENDS rl.net
    SOURCES ${RL_NET_CLI_SOURCES})
endif()