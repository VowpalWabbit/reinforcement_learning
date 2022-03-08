@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %rlRoot%

ECHO Building %vwBinaryParserRoot% for Release x64
REM x64-windows-static-md is a community triplet that sets CRT to dynamic but all the other dependencies to static
REM using -Wno-deprecated because zstd's CMakeLists.txt produces a deprecated warning that causes the CI to fail

cmake -S external_parser -B build -DCMAKE_TOOLCHAIN_FILE=%VcpkgCmake% -Wno-deprecated -DVCPKG_TARGET_TRIPLET="x64-windows-static" -G "Visual Studio 15 2017" -A x64 -DWARNING_AS_ERROR=OFF -DWARNINGS=OFF
cmake --build build --config Release --target vw-bin binary_parser_unit_tests

POPD
POPD

ENDLOCAL
