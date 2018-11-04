@echo off

set workspaceDir=%~1
set gameDir=%~2
set serverDir=%~3
set modName=%~4
set missionName=%~5

set tempFolder=%workspaceDir%\BIN

set makePBO=%workspaceDir%\Tools\MakePbo.exe
set signFile=%workspaceDir%\Tools\DSSignFile.exe

set keysLoc=%workspaceDir%\..\Keys

set privateKey=JacobMango.biprivatekey
set publicKey=JacobMango.bikey

echo "Clearing current mods"

RD /s /q "%tempFolder%" > NUL
RD /s /q "%gameDir%\%modName%" > NUL
RD /s /q "%serverDir%\%modName%" > NUL

echo "Packaging Addons"
xcopy /s/e /y /i %workspaceDir%\%modName% %tempFolder% > NUL
for /D %%s in (%tempFolder%\Addons\*) do (
    %makePBO% -U -P -D -N "%%s" "%%s.pbo"
    %signFile% "%keysLoc%\%privateKey%" "%%s.pbo"
    RD /s /q "%%s" > NUL
)
xcopy /s/e /y /i "%tempFolder%" "%gameDir%\%modName%" > NUL
xcopy /s/e /y /i "%tempFolder%" "%serverDir%\%modName%" > NUL

echo "Copying to DayZ Client"

RD /s /q "%gameDir%\Missions\%missionName%" > NUL
xcopy /s/e /y /i "%workspaceDir%\Missions\%missionName%" "%gameDir%\Missions\%missionName%" > NUL

echo "Copying to DayZ Server"

RD /s /q "%serverDir%\MPMissions\%missionName%" > NUL
xcopy /s/e /y /i "%workspaceDir%\Missions\%missionName%" "%serverDir%\MPMissions\%missionName%" > NUL

echo "Copying public key"
xcopy /y "%keysLoc%\%publicKey%" "%serverDir%\%modName%\Keys\%publicKey%*" > NUL

echo "Finished copying to DayZ Server"