if (rlclientlib_BUILD_DOTNET)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
  include(FindDotnet)

  if (rlclientlib_DOTNET_STRONG_NAME_SIGN)
    include(FindStrongName)

    get_target_property(rlclientlib_VW_CORE_SOURCE_ROOT vw SOURCE_DIR)
    set(rlclientlib_DOTNET_SIGNING_KEY_PATH "${rlclientlib_VW_CORE_SOURCE_ROOT}/../cs/vw_key.snk")

    configure_file(${rlclientlib_DOTNET_SIGNING_KEY_PATH} vw_key.snk COPYONLY)
    set(rlclientlib_DOTNET_SIGNING_KEY ${CMAKE_CURRENT_BINARY_DIR}/vw_key.snk)
    execute_process(COMMAND ${DOTNET_SN_COMMAND} -q -p ${rlclientlib_DOTNET_SIGNING_KEY} ${CMAKE_CURRENT_BINARY_DIR}/vw_pub.snk                    )
    execute_process(COMMAND ${DOTNET_SN_COMMAND} -q -tp ${CMAKE_CURRENT_BINARY_DIR}/vw_pub.snk
                    OUTPUT_VARIABLE rlclientlib_DOTNET_SIGNING_PUBLIC_INFO)

    string(REGEX REPLACE "^Public key \\(hash algorithm: sha1\\):[\r]?\n([A-Fa-f0-9\r\n]+)[\r]?\n[\r]?\nPublic key token is [A-Fa-f0-9]+.*$" "\\1"
           rlclientlib_DOTNET_SIGNING_PUBLIC_KEY ${rlclientlib_DOTNET_SIGNING_PUBLIC_INFO})

    string(REGEX REPLACE "[\r\n]" "" rlclientlib_DOTNET_SIGNING_PUBLIC_KEY ${rlclientlib_DOTNET_SIGNING_PUBLIC_KEY})

    set(rlclientlib_SIGNING_INCLUDE "/p:PackagingIntegration=${CMAKE_CURRENT_LIST_DIR}/rl.net.props;AssemblyOriginatorKeyFile=${CMAKE_CURRENT_BINARY_DIR}/vw_key.snk;PublicKey=${rlclientlib_DOTNET_SIGNING_PUBLIC_KEY};SignAssembly=True")
  else()
    set(rlclientlib_DOTNET_SIGNING_KEY "")
    set(rlclientlib_SIGNING_INCLUDE "")
  endif()

  add_subdirectory(rl.net.native)
  add_subdirectory(rl.net)
  add_subdirectory(rl.net.cli)
  add_subdirectory(rl.net.cli.test)

endif()
