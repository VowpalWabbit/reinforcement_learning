@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0

REM x64-windows-static-md is a community triplet that sets CRT to dynamic but all the other dependencies to static
%vcpkgPath% install zlib:x64-windows-static-md
%vcpkgPath% install flatbuffers:x64-windows-static-md
%vcpkgPath% install boost-filesystem:x64-windows-static-md
%vcpkgPath% install boost-thread:x64-windows-static-md
%vcpkgPath% install boost-program-options:x64-windows-static-md
%vcpkgPath% install boost-test:x64-windows-static-md
%vcpkgPath% install boost-align:x64-windows-static-md

POPD

ENDLOCAL
