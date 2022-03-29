@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init-cmake.cmd
PUSHD %rlRoot%

cmake -S . -B build -A "x64" -DCMAKE_TOOLCHAIN_FILE="%VcpkgCmake%" -DVCPKG_TARGET_TRIPLET=x64-windows-v141
cmake --build build --config Release

POPD

ENDLOCAL