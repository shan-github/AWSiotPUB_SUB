@echo off
set LOGFILE=awsInit.log
call :LOG > %LOGFILE%
exit /B

:LOG
echo Initializing esp8266...
mos aws-iot-setup --aws-region us-west-2

