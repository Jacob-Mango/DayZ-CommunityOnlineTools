@echo off

if "%~1"=="" (
	echo ERROR: Base directory argument not given^!
	echo Usage: %~nx0 ^<directory^>
	exit /b 1
)

if not exist "%~1" (
	echo ERROR: Directory "%~1" does not exist^!
	echo Usage: %~nx0 ^<directory^>
	exit /b 1
)

if exist "%~1\*.rpt" del /s /q /f "%~1\*.rpt"
if exist "%~1\*.log" del /s /q /f "%~1\*.log"
if exist "%~1\*.mdmp" del /s /q /f "%~1\*.mdmp"
if exist "%~1\*.ADM" del /s /q /f "%~1\*.ADM"
if exist "%~1\*.csv" del /q /f "%~1\*.csv"
