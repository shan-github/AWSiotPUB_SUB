@echo off
set LOGFILE=console.log
call :LOG > %LOGFILE%
exit /B

:LOG
title mos console
mos console