@echo off
setlocal

CALL "%~dp0SetupLaunch.bat"

if not defined AdditionalSPMods (
    echo AdditionalSPMods parameter was not set in the project.cfg, ignoring.
) else (
    set mods=%mods%;%AdditionalSPMods%
)

CALL "%~dp0SetupModList.bat"

@echo on
start /D "%gameDirectory%" %clientEXE% %clientLaunchParams% "-mod=%modList%" "-profiles=%ClientProfileDirectory%" "-name=%playerName%" "-mission=%SPMission%"
@echo off
