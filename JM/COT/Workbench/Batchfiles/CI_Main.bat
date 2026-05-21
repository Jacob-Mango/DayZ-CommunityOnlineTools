@echo off
setlocal

set modPrefix=%~n0
set modPrefix=%modPrefix:~3%

set exitCmd=0

:OPTIONS
REM parse command line options
if "%~1"=="" goto MAIN
if /I "%~1"=="exitCmd" (
	echo Command line option exitCmd found - will close host cmd.exe after running
	set exitCmd=1
)
shift /1
goto OPTIONS

:MAIN
if not exist "%~dp0..\Logs" mkdir "%~dp0..\Logs"

where tee > nul 2>&1
if errorlevel 1 (
	REM tee not available, just redirect to logfile
	echo Please wait, working...
	call "%~dp0CI1.bat" %modPrefix% %* > "%~dp0..\Logs\%modPrefix%.log" 2>&1
) else (
	REM tee available
	call "%~dp0CI1.bat" %modPrefix% %* | tee "%~dp0..\Logs\%modPrefix%.log"
)
set exitcode=0
if exist "%~dp0%modPrefix%.failure" set exitcode=1
if not exist "%~dp0%modPrefix%.success" set exitcode=1
if %exitCmd%==0 exit /B %exitcode%
exit %exitcode%
