@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0
PUSHD %vwBinaryParserRoot%

ECHO Building %vwBinaryParserRoot% for Release x64
REM x64-windows-static-md is a community triplet that sets CRT to dynamic but all the other dependencies to static
REM using -Wno-deprecated because zstd's CMakeLists.txt produces a deprecated warning that causes the CI to fail
cmake . -DCMAKE_TOOLCHAIN_FILE=%VcpkgCmake% -Wno-deprecated -DVCPKG_TARGET_TRIPLET=x64-windows-static-md-v141 -G "Visual Studio 16 2019" -A x64 -T v141 -DCMAKE_CONFIGURATION_TYPES="Release" -DWARNING_AS_ERROR=OFF -DWARNINGS=OFF
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build build --config Release -t vw-bin binary_parser_unit_tests
if %errorlevel% neq 0 exit /b %errorlevel%

POPD
POPD

ENDLOCAL
