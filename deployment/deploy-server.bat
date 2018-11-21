@echo off

set workspaceDir=%~dp0\..
set modName=%~1
set otherMods=%~2

set tempFolder=%workspaceDir%\BIN

set makePBO=%workspaceDir%\Tools\MakePbo.exe
set signFile=%workspaceDir%\Tools\DSSignFile.exe

set keysLoc=%workspaceDir%\..\Keys

set privateKey=JacobMango.biprivatekey
set publicKey=JacobMango.bikey

echo "Clearing current mods"

RD /s /q "%tempFolder%" > NUL

echo "Packaging Addons"
xcopy /s/e /y /i "%workspaceDir%\%modName%" "%tempFolder%" > NUL
for /D %%s in ("%tempFolder%\Addons\*") do (
    %makePBO% -U -P -D -N "%%s" "%%s.pbo"
    %signFile% "%keysLoc%\%privateKey%" "%%s.pbo"
    RD /s /q "%%s" > NUL
)
xcopy /s/e /y /i "%tempFolder%" "\\DESKTOP-TMCUVSS\Sharing\%modName%" > NUL

Powershell.exe -File deploy.ps1

echo "Finished copying to DayZ Server"
