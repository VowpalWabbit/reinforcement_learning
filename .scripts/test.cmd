@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd

REM TODO: Determine how to pass test failure out of this script so it can be used by CI/CD setups

ECHO Running RL Unit Tests
"%rlRoot%\x64\Release\unit_test.exe"

ECHO Running RL.Net Unit Tests
"%vstestPath%" /Platform:x64 /InIsolation "%rlRoot%\x64\Release\Rl.Net.Cli.Test.dll" /TestCaseFilter:"TestCategory!=NotOnVSO"

ENDLOCAL