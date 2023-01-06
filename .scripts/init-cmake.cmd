REM Integration points for toolchain customization
IF NOT DEFINED msbuildPath (
    CALL %~dp0find-vs2017.cmd
)

IF NOT DEFINED msbuildPath (
    IF NOT EXIST "%VsInstallDir%\MSBuild\15.0\Bin\MSBuild.exe" (
        IF EXIST "%VsInstallDir%\MSBuild\Current\Bin\MSBuild.exe" (
            SET "msBuildPath=%VsInstallDir%\MSBuild\Current\Bin\MSBuild.exe"
        ) else (
            ECHO ERROR: MsBuild couldn't be found
            EXIT /b 1
        )
    ) ELSE (
        SET "msBuildPath=%VsInstallDir%\MSBuild\15.0\Bin\MSBuild.exe"
    )
)

IF NOT DEFINED dotnetPath (
    SET dotnetPath=dotnet
)

IF NOT DEFINED VCPKG_ROOT (
    ECHO ERROR: VCPKG_ROOT is not configured. Cannot find vcpkg.
    EXIT /b 1
)

IF NOT DEFINED VCPKG_DEFAULT_BINARY_CACHE (
    ECHO ERROR: VCPKG_DEFAULT_BINARY_CACHE is not configured. Cannot find vcpkg binary installation location.
    EXIT /b 1
)

SET "vcpkgPath=%VCPKG_ROOT%\vcpkg.exe"
SET "vcpkgInstalledDir=%VCPKG_DEFAULT_BINARY_CACHE%"
SET "vcpkgCmake=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"

REM Repo-specific paths
IF NOT DEFINED rlRoot (
    SET rlRoot=%~dp0..
)

REM Repo-specific paths
IF NOT DEFINED vwBinaryParserRoot (
    SET vwBinaryParserRoot=%~dp0..\external_parser
)
