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

    cd %batchFileDirectory%
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

if %failed%==1 (
    endlocal

    echo Failed to package the mod.
    goto:eof
)

REM call CopyProject.bat

chdir /D "%workbenchDirectory%"
echo start %workbenchEXE% -gproj=%workDrive%%prefixLinkRoot%\Workbench\dayz.gproj
start %workbenchEXE% -gproj=%workDrive%%prefixLinkRoot%\Workbench\dayz.gproj

endlocal