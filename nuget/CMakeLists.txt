# Check that variables are set
if(NOT DEFINED RL_NUGET_PACKAGE_NAME OR NOT DEFINED RL_NUGET_PACKAGE_VERSION OR NOT DEFINED NATIVE_NUGET_PLATFORM_TAG)
  message(FATAL_ERROR "When building Nuget package, must define variables: RL_NUGET_PACKAGE_NAME, RL_NUGET_PACKAGE_VERSION, NATIVE_NUGET_PLATFORM_TAG")
endif()

# Generate NuGet package specification file from template
configure_file(rlclientlib.nuspec.in rlclientlib.nuspec @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rlclientlib.nuspec DESTINATION ./)

# Generate the .targets file from template
configure_file(rlclientlib.targets.in rlclientlib.targets @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rlclientlib.targets DESTINATION ./ RENAME ${RL_NUGET_PACKAGE_NAME}-v${MSVC_TOOLSET_VERSION}-${NATIVE_NUGET_PLATFORM_TAG}.targets)

# Copy OpenSSL libraries
# Extract library paths from lists with format
#   debug;/path/to/debug/libcrypto.lib;optimized;/path/to/optimized/libcrypto.lib
if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
  list(FIND OPENSSL_CRYPTO_LIBRARIES "debug" index_crypto)
  list(FIND OPENSSL_SSL_LIBRARIES "debug" index_ssl)
  math(EXPR index_crypto "${index_crypto} + 1")
  math(EXPR index_ssl "${index_ssl} + 1")
  list(GET OPENSSL_CRYPTO_LIBRARIES ${index_crypto} libcrypto_path)
  list(GET OPENSSL_SSL_LIBRARIES ${index_ssl} libssl_path)
  install(FILES ${libcrypto_path} DESTINATION ${CMAKE_INSTALL_LIBDIR} RENAME libcryptod.lib)
  install(FILES ${libssl_path} DESTINATION ${CMAKE_INSTALL_LIBDIR} RENAME libssld.lib)
else()
  list(FIND OPENSSL_CRYPTO_LIBRARIES "optimized" index_crypto)
  list(FIND OPENSSL_SSL_LIBRARIES "optimized" index_ssl)
  math(EXPR index_crypto "${index_crypto} + 1")
  math(EXPR index_ssl "${index_ssl} + 1")
  list(GET OPENSSL_CRYPTO_LIBRARIES ${index_crypto} libcrypto_path)
  list(GET OPENSSL_SSL_LIBRARIES ${index_ssl} libssl_path)
  install(FILES ${libcrypto_path} DESTINATION ${CMAKE_INSTALL_LIBDIR} RENAME libcrypto.lib)
  install(FILES ${libssl_path} DESTINATION ${CMAKE_INSTALL_LIBDIR} RENAME libssl.lib)
endif()

# Build package
install(SCRIPT CreateNugetPackage.cmake)