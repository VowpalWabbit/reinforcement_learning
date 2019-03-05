@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd

PUSHD %rlRoot%

REM TODO: Determine how to pass test failure out of this script so it can be used by CI/CD setups

ECHO Running RL Unit Tests
"%rlRoot%\x64\Release\unit_test.exe"

ECHO Running RL.Net Unit Tests
REM %vstestPath% /Platform:x64 /inIsolation "%vwRoot%\vowpalwabbit\x64\Release\cs_unittest.dll" /TestCaseFilter:"TestCategory!=NotOnVSO"

POPD

ENDLOCAL