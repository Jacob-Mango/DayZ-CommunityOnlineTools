@echo off

set gameDir=%~1
set serverDir=%~2
set modName=%~3
set missionName=%~4

set multiplayer=0

set mods="-mod=!Workshop\@RPCFramework;%modName%"

set profiles=%serverDir%\profiles\Server

taskkill /F /IM DayZ_x64.exe /T
taskkill /F /IM DayZServer_x64.exe /T

TIMEOUT /T 1 /NOBREAK

chdir /c "%workspaceDir%"
CALL deploy.bat %1 %2 %3 %4

IF %multiplayer%==1 (
    chdir /d "%serverDir%"
    start DayZServer_x64.exe -scrAllowFileWrite -config=serverDZ.cfg -port=2302 "-profiles=%profiles%" -dologs -adminlog -freezecheck -scriptDebug=true -cpuCount=4 -mission=.\MPMissions\%missionName% %mods%

    TIMEOUT /T 2 /NOBREAK

    chdir /d "%gameDir%"
    start DayZ_BE.exe -exe DayZ_x64.exe -password=abc123 -connect=127.0.0.1 -port=2302 -noPause -noBenchmark -scriptDebug=true -name=Jacob_Mango -freezecheck %mods%
) ELSE (
    chdir /d "%gameDir%"
    start DayZ_BE.exe -exe DayZ_x64.exe -noPause -noBenchmark -dologs -adminlog -netlog -scriptDebug=true -name=Jacob_Mango -freezecheck -mission=.\Missions\%missionName% %mods%
)