@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0\..\

REM x64-windows-static-md is a community triplet that sets CRT to dynamic but all the other dependencies to static
%vcpkgPath% install "--overlay-triplets=custom-triplets" --triplet=x64-windows-static-md-v141 --host-triplet=x64-windows-static-md-v141 zlib flatbuffers boost-filesystem boost-thread boost-program-options boost-test boost-align boost-math

if %errorlevel% neq 0 exit /b %errorlevel%

POPD

ENDLOCAL
