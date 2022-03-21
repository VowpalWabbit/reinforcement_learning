@ECHO OFF
IF DEFINED DebugBuildScripts (
    @ECHO ON
)

SETLOCAL

CALL %~dp0init.cmd
PUSHD %~dp0

SET "flatcPath=%VCPKG_INSTALLATION_ROOT%\installed\x64-windows-v141\tools\flatbuffers\flatc.exe"

REM The correct way to integrate this is to use msbuild targets and contribute them back to flatbuff,
REM rather than a command line execution
IF NOT DEFINED flatcPath (
    IF NOT DEFINED VcpkgInstallRoot (
        ECHO ERROR: flatcPath is not configured, but VcpkgInstallRoot is also not configured. Cannot find vcpkg.
        EXIT /b 1
    )

    SET "flatcPath=%VcpkgInstallRoot%installed\x64-windows-v141\tools\flatbuffers\flatc.exe"
)

REM TODO: Figure out how to parametrize this script?! (is there a standard, or do we actually need parse args?)
ECHO Building "%rlRoot%\rl.sln" for Release x64
"%msbuildPath%" /verbosity:normal /m /p:Configuration=Release;Platform=x64;VcpkgTriplet=x64-windows-v141 "%rlRoot%\rl.sln"

POPD

ENDLOCAL