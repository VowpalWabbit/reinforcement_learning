if (rlclientlib_BUILD_DOTNET)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
  include(FindDotnet)

  add_subdirectory(rl.net.native)

  add_custom_target(rl.net
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR}/rl.net -o $<TARGET_FILE_DIR:rl.net.native> -v m --nologo /clp:NoSummary
    COMMENT Building rl.net
    DEPENDS rl.net.native)

  add_custom_target(rl.net.cli
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR}/rl.net.cli -o $<TARGET_FILE_DIR:rl.net.native> -v m --nologo /clp:NoSummary
    COMMENT Building rl.net.cli
    DEPENDS rl.net)

  # No need to add the other two targets to ALL, because the rl.net.cli.test target will chain-build them
  add_custom_target(rl.net.cli.test ALL
    COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR}/rl.net.cli.test -o $<TARGET_FILE_DIR:rl.net.native> -v m --nologo /clp:NoSummary
    COMMENT Building rl.net.cli.test
    DEPENDS rl.net.cli)

  # TODO: Enable TRX test logging
  add_test(
    NAME rl.net.cli.test
    COMMAND
     ${DOTNET_COMMAND} test $<TARGET_FILE_DIR:rl.net.native>/rl.net.cli.test.dll --Platform:x64 --InIsolation --logger:console;verbosity=default 
    )
endif()
