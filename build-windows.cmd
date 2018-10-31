@echo off
setlocal

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set InstallDir=%%i
)

if not exist "%InstallDir%\MSBuild\15.0\Bin\MSBuild.exe" (
  echo ERROR: MsBuild couldn't be found
  exit /b 1
)

if not defined VcpkgInstallRoot (
  echo ERROR: VcpkgInstallRoot must be set
  exit /b 1
)

call "%InstallDir%\Common7\Tools\VsDevCmd.bat"

set VcpkgIntegration=%VcpkgInstallRoot%scripts\buildsystems\msbuild\vcpkg.targets

vcpkg install cpprestsdk:x64-windows

REM Need to install nuget packages before Visual Studio starts to make ANTLR targets available.
nuget install -o packages ext_libs\vowpal_wabbit\vowpalwabbit\packages.config
nuget install -o packages bindings\cs\rl.net.native\packages.config
nuget install -o packages rlclientlib\packages.config
nuget install -o packages unit_tests\packages.config

dotnet restore rl.sln

"%InstallDir%\MSBuild\15.0\Bin\MSBuild.exe" "rl.sln" /m /verbosity:normal /p:Configuration=Debug;Platform=x64
