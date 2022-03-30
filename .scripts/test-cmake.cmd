@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init-cmake.cmd
PUSHD %rlRoot%\build

ctest --verbose --output-on-failure -C Release
if %errorlevel% neq 0 exit /b %errorlevel%

ENDLOCAL