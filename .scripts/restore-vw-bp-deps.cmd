@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0

%vcpkgPath% install zlib:x64-windows-static
%vcpkgPath% install flatbuffers:x64-windows-static
%vcpkgPath% install boost-filesystem:x64-windows-static
%vcpkgPath% install boost-thread:x64-windows-static
%vcpkgPath% install boost-program-options:x64-windows-static
%vcpkgPath% install boost-test:x64-windows-static
%vcpkgPath% install boost-align:x64-windows-static
%vcpkgPath% install boost-math:x64-windows-static

POPD

ENDLOCAL
