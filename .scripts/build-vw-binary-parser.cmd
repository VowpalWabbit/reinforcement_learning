@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0
PUSHD %vwBinaryParserRoot%

ECHO Building %vwBinaryParserRoot% for Release x64
cmake . -DCMAKE_TOOLCHAIN_FILE=%VcpkgCmake% -DVCPKG_TARGET_TRIPLET=x64-windows -G "Visual Studio 15 2017" -A x64 -DBUILD_FLATBUFFERS=OFF -DCMAKE_CONFIGURATION_TYPES="Release" -DWARNINGS=OFF -DWARNINGS=OFF -DWARNING_AS_ERROR=OFF -DDO_NOT_BUILD_VW_C_WRAPPER=OFF  -DBUILD_JAVA=OFF -DBUILD_PYTHON=OFF -DBUILD_TESTS=OFF -DBUILD_EXPERIMENTAL_BINDING=OFF
"%msbuildPath%" /verbosity:normal /m /p:Configuration=Release;Platform=x64 "%vwBinaryParserRoot%\vw_binary_parser.sln"

POPD
POPD

ENDLOCAL