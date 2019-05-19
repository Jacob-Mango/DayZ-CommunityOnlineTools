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

    cd %batchFileDirectory%
    goto:eof
)

set workbenchDirectory=
set workbenchEXE=
set workDrive=
set prefixLinkRoot=
set modBuildDirectory=
set modName=
set mods=

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg ModName') do (
    set modName=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg AdditionalSPMods') do (
    set mods=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg ModBuildDirectory') do (
	set modBuildDirectory=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg WorkbenchDirectory') do (
    set workbenchDirectory=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg WorkbenchEXE') do (
    set workbenchEXE=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg WorkDrive') do (
	set workDrive=%%a
)

for /f "delims=" %%a in ('call ExtractData.bat ../project.cfg ../user.cfg PrefixLinkRoot') do (
    set prefixLinkRoot=%%a
)

setlocal enableextensions enabledelayedexpansion

echo WorkbenchEXE is: "%workbenchEXE%"
if "%workbenchEXE%"=="" (
    set /a failed=1
    echo WorkbenchEXE parameter was not set in the project.cfg
)

echo WorkbenchDirectory is: "%workbenchDirectory%"
if "%workbenchDirectory%"=="" (
    set /a failed=1
    echo WorkbenchDirectory parameter was not set in the project.cfg
)

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

echo ModName is: "%modName%"
if "%modName%"=="" (
    set /a failed=1
    echo ModName parameter was not set in the project.cfg
)

echo AdditionalSPMods is: "%mods%"
if "%mods%"=="" (
    echo AdditionalSPMods parameter was not set in the project.cfg, ignoring.
    
    set mods=%modName%
) else (
    set mods=%mods%;%modName%
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

REM call CopyProject.bat

for %%a in ("%mods:;=" "%") do (
    set mod=%%~a
    if not defined modList (
        set modList=%modBuildDirectory%!mod!
    ) else (
        set modList=!modList!;%modBuildDirectory%!mod!
    )
)

chdir /D "%workbenchDirectory%"

echo start %workbenchEXE% -gproj=%workDrive%%prefixLinkRoot%\Scripts\Workbench\dayz.gproj
start %workbenchEXE% -gproj=%workDrive%%prefixLinkRoot%\Scripts\Workbench\dayz.gproj

REM echo start %workbenchEXE% "-mod=%modList%"
REM start %workbenchEXE% "-mod=%modList%"

REM echo start %workbenchEXE% 
REM start %workbenchEXE%

REM echo start %workbenchEXE% -filePatching "-mod=P:\JM\CF\Scripts;P:\JM\COT\Scripts"
REM start %workbenchEXE% -filePatching "-mod=P:\JM\CF\Scripts;P:\JM\COT\Scripts"

endlocal