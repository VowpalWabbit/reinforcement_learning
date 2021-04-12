@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0

%vcpkgPath% install boost-filesystem:x64-windows
%vcpkgPath% install boost-thread:x64-windows
%vcpkgPath% install boost-program-options:x64-windows
%vcpkgPath% install boost-test:x64-windows

POPD

ENDLOCAL