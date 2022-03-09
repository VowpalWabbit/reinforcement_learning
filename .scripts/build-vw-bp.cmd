@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0
PUSHD %vwBinaryParserRoot%

SET "flatcPath=%VCPKG_INSTALLATION_ROOT%\installed\x64-windows-static-md-v141\tools\flatbuffers\flatc.exe"

REM The correct way to integrate this is to use msbuild targets and contribute them back to flatbuff,
REM rather than a command line execution
IF NOT DEFINED flatcPath (
    IF NOT DEFINED VcpkgInstallRoot (
        ECHO ERROR: flatcPath is not configured, but VcpkgInstallRoot is also not configured. Cannot find vcpkg.
        EXIT /b 1
    )

    SET "flatcPath=%VcpkgInstallRoot%installed\x64-windows-static-md-v141\tools\flatbuffers\flatc.exe"
)

ECHO Building %vwBinaryParserRoot% for Release x64
REM x64-windows-static-md is a community triplet that sets CRT to dynamic but all the other dependencies to static
REM using -Wno-deprecated because zstd's CMakeLists.txt produces a deprecated warning that causes the CI to fail
cmake . -DCMAKE_TOOLCHAIN_FILE=%VcpkgCmake% -Wno-deprecated -DVCPKG_TARGET_TRIPLET=x64-windows-static-md-v141 -G "Visual Studio 16 2019" -A x64 -T v141 -DCMAKE_CONFIGURATION_TYPES="Release" -DWARNING_AS_ERROR=OFF -DWARNINGS=OFF --debug-find
"%msbuildPath%" /verbosity:normal /m /p:Configuration=Release;Platform=x64 "%vwBinaryParserRoot%\vw_binary_parser.sln"

POPD
POPD

ENDLOCAL
