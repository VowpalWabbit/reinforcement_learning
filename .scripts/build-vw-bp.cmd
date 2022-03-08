@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0
PUSHD %vwBinaryParserRoot%

ECHO Building %vwBinaryParserRoot% for Release x64
REM using -Wno-deprecated because zstd's CMakeLists.txt produces a deprecated warning that causes the CI to fail
cmake . -DCMAKE_TOOLCHAIN_FILE=%VcpkgCmake% -Wno-deprecated -DVCPKG_TARGET_TRIPLET=x64-windows -G "Visual Studio 15 2017" -A x64 -DCMAKE_CONFIGURATION_TYPES="Release" -DWARNING_AS_ERROR=OFF -DWARNINGS=OFF
"%msbuildPath%" /verbosity:normal /m /p:Configuration=Release;Platform=x64 "%vwBinaryParserRoot%\vw_binary_parser.sln"

POPD
POPD

ENDLOCAL
