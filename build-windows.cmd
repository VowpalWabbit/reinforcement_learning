@echo off
setlocal

.scripts\init.cmd

REM TODO: Is this necessary?
call "%InstallDir%\Common7\Tools\VsDevCmd.bat"

.scripts\restore.cmd

.scripts\build.cmd
