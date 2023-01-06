@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init-cmake.cmd
PUSHD %rlRoot%

cmake -S . -B build -A "x64" -DCMAKE_TOOLCHAIN_FILE="%vcpkgCmake%" -DVCPKG_INSTALLED_DIR="%vcpkgInstalledDir%" -DVCPKG_TARGET_TRIPLET=x64-windows-v141 -DVCPKG_HOST_TRIPLET=x64-windows-v141 -DVCPKG_MANIFEST_MODE=Off
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build build --config Release
if %errorlevel% neq 0 exit /b %errorlevel%

POPD

ENDLOCAL