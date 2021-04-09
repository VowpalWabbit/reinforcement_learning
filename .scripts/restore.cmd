@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0

%vcpkgPath% install cpprestsdk:x64-windows
%vcpkgPath% install flatbuffers:x64-windows
%vcpkgPath% install openssl-windows:x64-windows
%vcpkgPath% install boost-filesystem:x64-windows

REM TODO: This really should be out-of-source (also, can we switch to vcpkg for these?)
ECHO Restoring "%rlRoot%\ext_libs\vowpal_wabbit\vowpalwabbit\packages.config"

REM Do not remove the space at the end of the "solutionDir" specifier, as otherwise it breaks NuGet
"%nugetPath%" install -o "%rlRoot%\packages" "%rlRoot%\ext_libs\vowpal_wabbit\vowpalwabbit\packages.config" -solutionDir "%rlRoot%\ "
ECHO.

ECHO Restoring "%rlRoot%\bindings\cs\rl.net.native\packages.config"
"%nugetPath%" install -o "%rlRoot%\packages" "%rlRoot%\bindings\cs\rl.net.native\packages.config"
ECHO.

ECHO Restoring "%rlRoot%\rlclientlib\packages.config"
"%nugetPath%" install -o "%rlRoot%\packages" "%rlRoot%\rlclientlib\packages.config"
ECHO.

ECHO Restoring "%rlRoot%\unit_tests\packages.config"
"%nugetPath%" install -o "%rlRoot%\packages" "%rlRoot%\unit_tests\packages.config"
ECHO.

ECHO Restoring SDK-Style projects in "%rlRoot%\rl.sln"
%dotnetPath% restore "%rlRoot%\rl.sln"
ECHO.

ECHO Restoring DotNet Tools (made simpler in .NET Core 3.0)
%dotnetPath% tool install -g dotnet-t4 --version 2.0.5

POPD

ENDLOCAL