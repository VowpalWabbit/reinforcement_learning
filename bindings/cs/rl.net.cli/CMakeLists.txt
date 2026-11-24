set (RL_NET_CLI_SOURCES
  BasicUsageCommand.cs
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
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RL_DOTNET_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:rlnetnative>/rl.net.dll ${RL_DOTNET_OUTPUT_DIRECTORY}
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} --framework ${RL_DOTNET_TARGET_FRAMEWORK} -o ${RL_DOTNET_OUTPUT_DIRECTORY} -v m --nologo --no-dependencies /clp:NoSummary --configuration "$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>"
    COMMENT Building rl.net.cli
    DEPENDS rl.net
    SOURCES ${RL_NET_CLI_SOURCES})
endif()