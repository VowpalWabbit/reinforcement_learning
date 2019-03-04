@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0

%vcpkgPath% install cpprestsdk:x64-windows
%vcpkgPath% install flatbuffers:x64-windows

REM TODO: This really should be out-of-source (also, can we switch to vcpkg for these?)
ECHO Restoring "%rlRoot%\ext_libs\vowpal_wabbit\vowpalwabbit\packages.config"

REM Do not remove the space at the end of the "solutionDir" specifier, as otherwise it breaks NuGet
"%nugetPath%" install -o packages "%rlRoot%\ext_libs\vowpal_wabbit\vowpalwabbit\packages.config" -solutionDir "%rlRoot%\ "
ECHO.

ECHO Restoring "%rlRoot%\bindings\cs\rl.net.native\packages.config"
"%nugetPath%" install -o packages "%rlRoot%\bindings\cs\rl.net.native\packages.config"
ECHO.

ECHO Restoring "%rlRoot%\ext_libs\vowpal_wabbit\vowpalwabbit\packages.config"
"%nugetPath%" install -o packages "%rlRoot%\rlclientlib\packages.config"
ECHO.

ECHO Restoring "%rlRoot%\ext_libs\vowpal_wabbit\vowpalwabbit\packages.config"
"%nugetPath%" install -o packages "%rlRoot%\unit_tests\packages.config"
ECHO.

ECHO Restoring SDK-Style projects in "%rlRoot%\rl.sln"
%dotnetPath% restore "%rlRoot%\rl.sln"
ECHO.

REM TODO: Is this still necessary?
SET PATH=%PATH%;%VcpkgInstallRoot%installed\x64-windows\tools\flatbuffers


POPD

ENDLOCAL