@ECHO ON
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init-cmake.cmd
PUSHD %~dp0\..\

REM must be in repo root dir for overlay-triplets to work
%vcpkgPath% install "--overlay-triplets=custom-triplets" --triplet=x64-windows-v141 --host-triplet=x64-windows-v141 zlib boost-system boost-program-options boost-test boost-align boost-foreach boost-math boost-uuid cpprestsdk flatbuffers openssl-windows

ECHO Restoring DotNet Tools (made simpler in .NET Core 3.0)
%dotnetPath% tool install -g dotnet-t4 --version 2.2.1

POPD

ENDLOCAL
