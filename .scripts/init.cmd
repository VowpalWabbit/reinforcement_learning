REM Integration points for toolchain customization
IF NOT DEFINED nugetPath (
    SET nugetPath=nuget
)

IF NOT DEFINED msbuildPath (
    REM Try to find VS Install
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
        set InstallDir=%%i
    )

    if not exist "%InstallDir%\MSBuild\15.0\Bin\MSBuild.exe" (
        echo ERROR: MsBuild couldn't be found
        exit /b 1
    ) else (
        SET "msBuildPath=%InstallDir%\MSBuild\15.0\Bin\MSBuild.exe"
    )
)

IF NOT DEFINED dotnetPath (
    SET dotnetPath=dotnet
)

IF NOT DEFINED vcpkgPath (
    IF NOT DEFINED VcpkgInstallRoot (
        ECHO ERROR: vcpkgPath is not configured, but VcpkgInstallRoot is also not configured. Cannot find vcpkg.
        EXIT /b 1
    )

    SET "vcpkgPath=%VcpkgInstallRoot%vcpkg.exe"
    SET "VcpkgIntegration=%VcpkgInstallRoot%scripts\buildsystems\msbuild\vcpkg.targets"
)

REM The correct way to integrate this is to use msbuild targets and contribute them back to flatbuff, 
REM rather than a command line execution
IF NOT DEFINED flatcPath (
    IF NOT DEFINED VcpkgInstallRoot (
        ECHO ERROR: flatcPath is not configured, but VcpkgInstallRoot is also not configured. Cannot find vcpkg.
        EXIT /b 1
    )

    SET "flatcPath=%VcpkgInstallRoot%installed\x64-windows\tools\flatbuffers\flatc.exe"
)

REM Repo-specific paths
IF NOT DEFINED rlRoot (
    SET rlRoot=%~dp0..
)