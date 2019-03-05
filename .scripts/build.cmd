@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0

REM It seems like the build needs to happen in rlRoot, though I am not super-sure why that is.
PUSHD %rlRoot% 

REM TODO: Figure out how to parametrize this script?! (is there a standard, or do we actually need parse args?)
ECHO Building "%rlRoot%\rl.sln" for Release x64
"%msbuildPath%" /verbosity:normal /m /p:Configuration=Release;Platform=x64 "%rlRoot%\rl.sln"

POPD

POPD

ENDLOCAL