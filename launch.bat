@echo off

set gameDir=%~1
set serverDir=%~2
set modName=%~3
set missionName=%~4

set multiplayer=1

set profiles=%serverDir%\profiles\Server

TIMEOUT /T 1 /NOBREAK

IF %multiplayer%==1 (
    taskkill /F /IM DayZ_x64.exe /T
    taskkill /F /IM DayZServer_x64.exe /T

    chdir /c "%workspaceDir%"
    CALL deploy.bat %1 %2 %3 %4

    chdir /d "%serverDir%"
    start DayZServer_x64.exe -scrAllowFileWrite -config=serverDZ.cfg -port=2302 "-profiles=%profiles%" -dologs -adminlog -freezecheck -scriptDebug=true -cpuCount=4 -mission=.\MPMissions\%missionName% "-mod=@RPCFramework;%modName%"

    TIMEOUT /T 2 /NOBREAK

    chdir /d "%gameDir%"
    start DayZ_BE.exe -exe DayZ_x64.exe -connect=127.0.0.1 -port=2302 -noPause -noBenchmark -scriptDebug=true -name=Jacob_Mango -freezecheck "-mod=!Workshop\@RPCFramework;%modName%"
) ELSE IF %multiplayer%==2 (
    Powershell.exe -File "%cd%\Tools\exit.ps1" @RPCFramework %modName%

    chdir /c "%workspaceDir%"
    CALL Tools/deploy-server.bat "%modName%" @RPCFramework %modName%
    
    Powershell.exe -File "%cd%\Tools\launch.ps1" "@RPCFramework %modName%"
    
    REM chdir /d "%gameDir%"
    REM start DayZ_BE.exe -exe DayZ_x64.exe -noPause -noBenchmark -dologs -adminlog -netlog -scriptDebug=true -name=Jacob_Mango -freezecheck -mission=.\Missions\%missionName% "-mod=!Workshop\@RPCFramework;%modName%"

    goto:eof
) ELSE (
    taskkill /F /IM DayZ_x64.exe /T

    chdir /c "%workspaceDir%"
    CALL deploy.bat %1 %2 %3 %4

    chdir /d "%gameDir%"
    start DayZ_BE.exe -exe DayZ_x64.exe -noPause -noBenchmark -dologs -adminlog -netlog -scriptDebug=true -name=Jacob_Mango -freezecheck -mission=.\Missions\%missionName% "-mod=!Workshop\@RPCFramework;%modName%"
)