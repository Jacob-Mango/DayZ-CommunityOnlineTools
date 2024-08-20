@echo off
setlocal

CALL "%~dp0SetupLaunch.bat"

if not defined AdditionalMPMods (
    echo AdditionalMPMods parameter was not set in the project.cfg, ignoring.
) else (
    set mods=%mods%;%AdditionalMPMods%
)

CALL "%~dp0SetupModList.bat"

call "%~dp0ClearLogs.bat" "%serverProfileDirectory%"

call "%~dp0LaunchSteamClient.bat" 1 "%serverDirectory%" %serverEXE% %serverLaunchParams% "-config=%serverConfig%" -port=%port% "-profiles=%serverProfileDirectory%" "-mission=%MPMission%" "-mod=%modList%"
