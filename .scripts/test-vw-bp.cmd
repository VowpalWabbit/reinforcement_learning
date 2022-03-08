@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %rlRoot%

ECHO Running vw binary parser unit tests
.\build\unit_tests\Release\binary_parser_unit_tests.exe

ENDLOCAL