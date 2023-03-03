@echo off

cd /D "%~dp0"

set /a failed=0

if exist ../project.cfg (
	echo Found the project.cfg
) else (
	echo Failed to find the project.cfg file, exitting.
	set /a failed=1
)

if exist ../user.cfg (
	echo Found the user.cfg
) else (
	echo Failed to find the user.cfg file, exitting.
	set /a failed=1
)

if %failed%==1 (
    endlocal

    echo Failed to package the mod.
    goto:eof
)

set workDrive=
set /a majorVersion=0
set /a minorVersion=0
set versionFileLocation=

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg WorkDrive') do (
    set workDrive=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg MajorVersion') do (
    set /a majorVersion=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg MinorVersion') do (
    set /a minorVersion=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg VersionFileLocation') do (
    set versionFileLocation=%%a
)

setlocal enableextensions enabledelayedexpansion

echo WorkDrive is: "%workDrive%"
if "%workDrive%"=="" (
    set /a failed=1
    echo WorkDrive parameter was not set in the project.cfg
)

echo VersionFileLocation is: "%versionFileLocation%"
if "%versionFileLocation%"=="" (
    set /a failed=1
    echo VersionFileLocation parameter was not set in the project.cfg
)

if %failed%==1 (
    endlocal

    echo Failed to package the mod.
    goto:eof
)

for /f "tokens=1-3 delims=." %%a in (%workDrive%%versionFileLocation%) do (
	set majorVersion=%%a
	set minorVersion=%%b
    echo MajorVersion was: %%a
    echo MinorVersion was: %%b
    echo BuildVersion was: %%c
    set /a build=%%c+1
	if !build! GTR 9999 (
		echo MinorVersion was: !minorVersion!
		set /a minorVersion+=1
		echo MinorVersion is: !minorVersion!
		set build=0
	)
    set version=!majorVersion!.!minorVersion!.!build!
    echo BuildVersion is: !build!
)

echo %version%>%workDrive%%versionFileLocation%
echo Version: %version%

endlocal