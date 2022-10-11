@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init-cmake.cmd
PUSHD %vwBinaryParserRoot%\build

ECHO Running vw binary parser unit tests
ctest --verbose --output-on-failure -C Release
if %errorlevel% neq 0 exit /b %errorlevel%

POPD

ENDLOCAL