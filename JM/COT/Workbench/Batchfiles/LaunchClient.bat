@echo off
setlocal

CALL "%~dp0SetupLaunch.bat"

set mods=%ModName%

if "%AdditionalMPMods%"=="" (
    echo AdditionalMPMods parameter was not set in the project.cfg, ignoring.
) else (
    set mods=%AdditionalMPMods%;%ModName%
)

CALL "%~dp0SetupModList.bat"

call "%~dp0ClearLogs.bat" "%ClientAProfileDirectory%"

@echo on
start /D "%gameDirectory%" %clientEXE% %clientLaunchParams% "-mod=%modList%" "-name=%playerName%"
