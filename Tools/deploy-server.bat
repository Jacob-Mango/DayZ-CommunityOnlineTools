@echo off

Powershell.exe -File "%cd%\Tools\exit.ps1"

set workspaceDir=%cd%
set modName=%~1

set OTHER_MODS=
shift
:loop
    set OTHER_MODS=%OTHER_MODS% "%1"
    shift

    if not "%~1"=="" goto loop

echo "Loading mods for server to copy %OTHER_MODS%"

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

echo "Transferring mods (%OTHER_MODS%) from share folder to server!"

powershell -command "%cd%\Tools\deploy.ps1 \"%OTHER_MODS%\""

echo "Finished copying to DayZ Server"
