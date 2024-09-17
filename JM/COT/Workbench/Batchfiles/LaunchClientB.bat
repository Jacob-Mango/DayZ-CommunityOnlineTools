@echo off
setlocal

CALL "%~dp0SetupLaunch.bat"

if not defined AdditionalMPMods (
    echo AdditionalMPMods parameter was not set in the project.cfg, ignoring.
) else (
    set mods=%mods%;%AdditionalMPMods%
)

CALL "%~dp0SetupModList.bat"

call "%~dp0ClearLogs.bat" "%ClientBProfileDirectory%"

call "%~dp0LaunchSteamClient.bat" %PlayerBSteamID% "%gameDirectory%" %clientEXE% %clientLaunchParams% "-connect=127.0.0.1" -port=%port% "-profiles=%ClientBProfileDirectory%" "-name=%playerBName%" %password% "-mod=%modList%"
