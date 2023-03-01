@echo off
setlocal enableextensions enabledelayedexpansion

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

if exist "%~dp0..\project.cfg.bat" del "%~dp0..\project.cfg.bat"

for /f "usebackq delims=" %%a in ( "%~dp0..\project.cfg" ) do (
	echo set %%a>>"%~dp0..\project.cfg.bat"
)

call "%~dp0..\project.cfg.bat"

if exist "%~dp0..\user.cfg.bat" del "%~dp0..\user.cfg.bat"

for /f "usebackq delims=" %%a in ( "%~dp0..\user.cfg" ) do (
	echo set %%a>>"%~dp0..\user.cfg.bat"
)

call "%~dp0..\user.cfg.bat"

echo ClientEXE is: "%clientEXE%"
if "%clientEXE%"=="" (
    set /a failed=1
    echo ClientEXE parameter was not set in the project.cfg
)

echo ClientLaunchParams is: "%clientLaunchParams%"
if "%clientLaunchParams%"=="" (
    echo ClientLaunchParams parameter was not set in the project.cfg
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

echo GameDirectory is: "%gameDirectory%"
if "%gameDirectory%"=="" (
    set /a failed=1
    echo GameDirectory parameter was not set in the project.cfg
)

echo PlayerName is: "%playerName%"
if "%playerName%"=="" (
    echo PlayerName parameter was not set in the project.cfg, continuing.
) else (
    set playerName=-name=%playerName%
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

CALL Exit.bat

for %%a in ("%mods:;=" "%") do (
    set mod=%%~a
    if not defined modList (
        set modList=%modBuildDirectory%!mod!
    ) else (
        set modList=!modList!;%modBuildDirectory%!mod!
    )
)

chdir /d "%gameDirectory%"
echo start "" "%clientEXE%" %clientLaunchParams% "-mod=%modList%" "-mission=%mission%" "%playerName%" -dologs -nopause -adminlog -freezecheck "-scriptDebug=true"
start "" "%clientEXE%" %clientLaunchParams% "-mod=%modList%" "-mission=%mission%" "%playerName%" -dologs -adminlog -freezecheck "-scriptDebug=true"

endlocal