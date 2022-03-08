@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd

REM x64-windows-static-md is a community triplet that sets CRT to dynamic but all the other dependencies to static
%vcpkgPath% install --triplet=x64-windows-static zlib flatbuffers boost-filesystem boost-thread boost-program-options boost-test boost-align boost-math

POPD

ENDLOCAL
