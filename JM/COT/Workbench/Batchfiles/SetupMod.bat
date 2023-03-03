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

set rootDirectory=%cd%\..\..\..\
set workDrive=
set workbenchWorkDrive=
set prefixLinkRoot=
set workbenchDirectory=
set gameDirectory=
set serverDirectory=
set modName=
set modBuildDirectory=

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg WorkDrive') do (
    set workDrive=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg WorkbenchWorkDrive') do (
    set workbenchWorkDrive=%%a
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

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg ModName') do (
	set modName=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg ModBuildDirectory') do (
	set modBuildDirectory=%%a
)

setlocal enableextensions enabledelayedexpansion

echo WorkDrive is: "%workDrive%"
if "%workDrive%"=="" (
    set /a failed=1
    echo WorkDrive parameter was not set in the project.cfg
)

echo WorkbenchWorkDrive is: "%workbenchWorkDrive%"
if "%workbenchWorkDrive%"=="" (
    set /a failed=1
    echo workbenchWorkDrive parameter was not set in the project.cfg
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

echo ModName is: "%modName%"
if "%modName%"=="" (
	set /a failed=1
	echo ModName parameter was not set in the project.cfg
)

echo ModBuildDirectory is: "%modBuildDirectory%"
if "%modBuildDirectory%"=="" (
	set /a failed=1
	echo ModBuildDirectory parameter was not set in the project.cfg
)


if %failed%==1 (
    endlocal

    echo Failed to package the mod.
    goto:eof
)

IF exist "%workDrive%%prefixLinkRoot%\" (
	echo Removing existing link "%workDrive%%prefixLinkRoot%\"
	rmdir "%workDrive%%prefixLinkRoot%\"
)

echo Creating link "%workDrive%%prefixLinkRoot%\"
mklink /J "%workDrive%%prefixLinkRoot%\" "%rootDirectory%%prefixLinkRoot%\"


IF exist "%workbenchWorkDrive%%prefixLinkRoot%\" (
	echo Removing existing link "%workbenchWorkDrive%%prefixLinkRoot%\"
	rmdir "%workbenchWorkDrive%%prefixLinkRoot%\"
)
echo Creating link "%workbenchWorkDrive%\"
mklink /J "%workbenchWorkDrive%%prefixLinkRoot%\" "%workDrive%%prefixLinkRoot%\"


IF exist "%workbenchWorkDrive%bin\" (
	echo Removing existing link "%workbenchWorkDrive%bin\"
	rmdir "%workbenchWorkDrive%bin\"
)
echo Creating link "%workbenchWorkDrive%bin\"
mklink /J "%workbenchWorkDrive%bin\" "%workDrive%bin\"


IF exist "%workbenchWorkDrive%Core\" (
	echo Removing existing link "%workbenchWorkDrive%Core\"
	rmdir "%workbenchWorkDrive%Core\"
)
echo Creating link "%workbenchWorkDrive%Core\"
mklink /J "%workbenchWorkDrive%Core\" "%workDrive%Core\"


IF exist "%workbenchWorkDrive%dta\" (
	echo Removing existing link "%workbenchWorkDrive%dta\"
	rmdir "%workbenchWorkDrive%dta\"
)
echo Creating link "%workbenchWorkDrive%dta\"
mklink /J "%workbenchWorkDrive%dta\" "%workDrive%dta\"


IF exist "%workbenchWorkDrive%DZ\" (
	echo Removing existing link "%workbenchWorkDrive%DZ\"
	rmdir "%workbenchWorkDrive%DZ\"
)
echo Creating link "%workbenchWorkDrive%DZ\"
mklink /J "%workbenchWorkDrive%DZ\" "%workDrive%DZ\"


IF exist "%workbenchWorkDrive%graphics\" (
	echo Removing existing link "%workbenchWorkDrive%graphics\"
	rmdir "%workbenchWorkDrive%graphics\"
)
echo Creating link "%workbenchWorkDrive%graphics\"
mklink /J "%workbenchWorkDrive%graphics\" "%workDrive%graphics\"


IF exist "%workbenchWorkDrive%gui\" (
	echo Removing existing link "%workbenchWorkDrive%gui\"
	rmdir "%workbenchWorkDrive%gui\"
)
echo Creating link "%workbenchWorkDrive%gui\"
mklink /J "%workbenchWorkDrive%gui\" "%workDrive%gui\"


IF exist "%workbenchWorkDrive%languagecore\" (
	echo Removing existing link "%workbenchWorkDrive%languagecore\"
	rmdir "%workbenchWorkDrive%languagecore\"
)
echo Creating link "%workbenchWorkDrive%languagecore\"
mklink /J "%workbenchWorkDrive%languagecore\" "%workDrive%languagecore\"


IF exist "%workbenchWorkDrive%scripts\" (
	echo Removing existing link "%workbenchWorkDrive%scripts\"
	rmdir "%workbenchWorkDrive%scripts\"
)
echo Creating link "%workbenchWorkDrive%scripts\"
mklink /J "%workbenchWorkDrive%scripts\" "%workDrive%scripts\"


IF exist "%workbenchWorkDrive%DayZEditor\" (
	echo Removing existing link "%workbenchWorkDrive%DayZEditor\"
	rmdir "%workbenchWorkDrive%DayZEditor\"
)
IF exist "%workDrive%DayZEditor\" (
    echo Creating link "%workbenchWorkDrive%DayZEditor\"
    mklink /J "%workbenchWorkDrive%DayZEditor\" "%workDrive%DayZEditor\"
)


IF exist "%workbenchWorkDrive%DabsFramework\" (
	echo Removing existing link "%workbenchWorkDrive%DabsFramework\"
	rmdir "%workbenchWorkDrive%DabsFramework\"
)
IF exist "%workDrive%DabsFramework\" (
    echo Creating link "%workbenchWorkDrive%DabsFramework\"
    mklink /J "%workbenchWorkDrive%DabsFramework\" "%workDrive%DabsFramework\"
)


IF exist "%workbenchWorkDrive%eAI\" (
	echo Removing existing link "%workbenchWorkDrive%eAI\"
	rmdir "%workbenchWorkDrive%eAI\"
)
IF exist "%workDrive%eAI\" (
    echo Creating link "%workbenchWorkDrive%eAI\"
    mklink /J "%workbenchWorkDrive%eAI\" "%workDrive%eAI\"
)

IF exist "%workbenchWorkDrive%Silens-Nox-Redux\" (
	echo Removing existing link "%workbenchWorkDrive%Silens-Nox-Redux\"
	rmdir "%workbenchWorkDrive%Silens-Nox-Redux\"
)
IF exist "%workDrive%Silens-Nox-Redux\" (
    echo Creating link "%workbenchWorkDrive%Silens-Nox-Redux\"
    mklink /J "%workbenchWorkDrive%Silens-Nox-Redux\" "%workDrive%Silens-Nox-Redux\"
)

IF exist "%workbenchWorkDrive%JM\" (
	echo Removing existing link "%workbenchWorkDrive%JM\"
	rmdir "%workbenchWorkDrive%JM\"
)
IF exist "%workDrive%JM\" (
    echo Creating link "%workbenchWorkDrive%JM\"
    mklink /J "%workbenchWorkDrive%JM\" "%workDrive%JM\"
)

endlocal