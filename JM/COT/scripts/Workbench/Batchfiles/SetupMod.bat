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

set rootDirectory=%cd%\..\..\..\..\
set workDrive=
set prefixLinkRoot=
set workbenchDirectory=
set gameDirectory=
set serverDirectory=

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg WorkDrive') do (
    set workDrive=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg PrefixLinkRoot') do (
	set prefixLinkRoot=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg GameDirectory') do (
    set gameDirectory=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg ServerDirectory') do (
    set serverDirectory=%%a
)

setlocal enableextensions enabledelayedexpansion

echo WorkDrive is: "%workDrive%"
if "%workDrive%"=="" (
    set /a failed=1
    echo WorkDrive parameter was not set in the project.cfg
)

echo PrefixLinkRoot is: "%prefixLinkRoot%"
if "%prefixLinkRoot%"=="" (
	set /a failed=1
	echo PrefixLinkRoot parameter was not set in the project.cfg
)

echo GameDirectory is: "%gameDirectory%"
if "%gameDirectory%"=="" (
    set /a failed=1
    echo GameDirectory parameter was not set in the project.cfg
)

echo ServerDirectory is: "%serverDirectory%"
if "%serverDirectory%"=="" (
    set /a failed=1
    echo ServerDirectory parameter was not set in the project.cfg
)

if %failed%==1 (
    endlocal

    echo Failed to package the mod.
    goto:eof
)

IF exist "%workDrive%%prefixLinkRoot%\" (
	echo Removing existing link %workDrive%%prefixLinkRoot%\
	rmdir "%workDrive%%prefixLinkRoot%\"
)

echo Creating link %workDrive%%prefixLinkRoot%\
mklink /J "%workDrive%%prefixLinkRoot%\" "%rootDirectory%%prefixLinkRoot%\"

set directory=%rootDirectory%\Missions\
for /f "tokens=*" %%D in ('dir /b "%directory%"') do (
    set missionFolder=%%~D

    IF exist "%gameDirectory%\Missions\!missionFolder!\" (
        echo.Please remove existing link: "%gameDirectory%\Missions\!missionFolder!\"
        REM rmdir /s /q "%gameDirectory%\Missions\!missionFolder!\"
    ) else (
        echo.Creating link "%gameDirectory%\Missions\!missionFolder!\"
        mklink /J "%gameDirectory%\Missions\!missionFolder!\" "%directory%\!missionFolder!\"
    )

    IF exist "%serverDirectory%\MPMissions\!missionFolder!\" (
        echo.Please remove existing link: "%serverDirectory%\MPMissions\!missionFolder!\"
        REM rmdir /s /q "%serverDirectory%\MPMissions\!missionFolder!\"
    ) else (
        echo.Creating link "%serverDirectory%\MPMissions\!missionFolder!\"
        mklink /J "%serverDirectory%\MPMissions\!missionFolder!\" "%directory%\!missionFolder!\"
    )
)

endlocal