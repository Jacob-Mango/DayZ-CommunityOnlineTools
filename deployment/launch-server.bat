\\DESKTOP-TMCUVSS\Sharing
C:\Program Files (x86)\Steam\steamapps\common\DayZServer

@echo off

set gameDir=%~1
set serverDir=%~2
set modName=%~3
set missionName=%~4

set multiplayer=1

set profiles=%serverDir%\profiles\Server

taskkill /F /IM DayZ_x64.exe /T
taskkill /F /IM DayZServer_x64.exe /T

TIMEOUT /T 1 /NOBREAK

chdir /c "%workspaceDir%"
CALL deploy.bat %1 %2 %3 %4

chdir /d "%serverDir%"

start DayZServer_x64.exe -scrAllowFileWrite -config=serverDZ.cfg -port=2302 "-profiles=%profiles%" -dologs -adminlog -freezecheck -scriptDebug=true -cpuCount=4 -mission=.\MPMissions\%missionName% "-mod=@RPCFramework;%modName%"