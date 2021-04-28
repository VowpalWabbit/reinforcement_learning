@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd

ECHO Running vw binary parser unit tests
"%vwBinaryParserRoot%\unit_tests\Release\binary_parser_unit_tests.exe"

ENDLOCAL