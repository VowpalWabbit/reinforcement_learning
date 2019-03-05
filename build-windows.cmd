@echo off
setlocal

CALL .scripts\init.cmd

REM TODO: Is this necessary?
call "%InstallDir%\Common7\Tools\VsDevCmd.bat"

CALL .scripts\restore.cmd

CALL .scripts\build.cmd
