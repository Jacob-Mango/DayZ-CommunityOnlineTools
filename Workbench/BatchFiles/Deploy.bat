@echo off

set /a failed=0

if exist ../../project.cfg (
    echo Found the project.cfg
    cd ../../
) else (
    echo project.cfg doesn't exist in first directory, trying the next possible directory.
    if exist project.cfg (
        echo Found the project.cfg
    ) else (
        echo Failed to find the project.cfg
        set /a failed=1
    )
)

if %failed%==1 (
    endlocal

    echo Failed to package the mod.

    cd %batchFileDirectory%
    goto:eof
)

set githubDirectory=%cd%\
set workbenchDataDirectory=%githubDirectory%Workbench\
set toolsDirectory=%workbenchDataDirectory%Tools\
set batchFileDirectory=%workbenchDataDirectory%BatchFiles\

set workDrive=
set gameDirectory=
set serverDirectory=
set modName=
set prefixLinkRoot=
set privateKey=
set publicKey=

for /f "delims=" %%a in ('call %batchFileDirectory%ExtractData.bat project.cfg WorkDrive') do (
    set workDrive=%%a
)

for /f "delims=" %%a in ('call %batchFileDirectory%ExtractData.bat project.cfg GameDirectory') do (
    set gameDirectory=%%a
)

for /f "delims=" %%a in ('call %batchFileDirectory%ExtractData.bat project.cfg ServerDirectory') do (
    set serverDirectory=%%a
)

for /f "delims=" %%a in ('call %batchFileDirectory%ExtractData.bat project.cfg ModName') do (
    set modName=%%a
)

for /f "delims=" %%a in ('call %batchFileDirectory%ExtractData.bat project.cfg PrefixLinkRoot') do (
    set prefixLinkRoot=%%a
)

for /f "delims=" %%a in ('call %batchFileDirectory%ExtractData.bat project.cfg PrivateKey') do (
    set privateKey=%%a
)

for /f "delims=" %%a in ('call %batchFileDirectory%ExtractData.bat project.cfg PublicKey') do (
    set publicKey=%%a
)

setlocal enableextensions enabledelayedexpansion

echo PrivateKey is: "%privateKey%"
if "%privateKey%"=="" (
    set /a failed=1
    echo PrivateKey parameter was not set in the project.cfg
)

echo PublicKey is: "%publicKey%"
if "%publicKey%"=="" (
    set /a failed=1
    echo PublicKey parameter was not set in the project.cfg
)

echo ModName is: "%modName%"
if "%modName%"=="" (
    set /a failed=1
    echo ModName parameter was not set in the project.cfg
)

echo WorkDrive is: "%workDrive%"
if "%workDrive%"=="" (
    set /a failed=1
    echo WorkDrive parameter was not set in the project.cfg
)

echo GameDirectory is: "%gameDirectory%"
if "%gameDirectory%"=="" (
    set /a failed=1
    echo GameDirectory parameter was not set in the project.cfg
)

echo ServerDirectory is: "%serverDirectory%"
if "%serverDirectory%"=="" (
    set /a failed=1
    echo ServerDirectory parameter was not set in the project.cfg
)

echo PrefixLinkRoot is: "%prefixLinkRoot%"
if "%prefixLinkRoot%"=="" (
    set /a failed=1
    echo PrefixLinkRoot parameter was not set in the project.cfg
)

if %failed%==1 (
    endlocal

    echo Failed to package the mod.

    cd %batchFileDirectory%
    goto:eof
)

set temporaryDirectory=%workDrive%temp\

set makePBO=%toolsDirectory%MakePbo.exe
set signFile=%toolsDirectory%DSSignFile.exe

echo "Packaging Addons"

rmdir /S /Q "%temporaryDirectory%%modName%\Addons\"

mkdir "%temporaryDirectory%%modName%\"
mkdir "%temporaryDirectory%%modName%\Addons\"
mkdir "%temporaryDirectory%%modName%\Keys\"

copy "%publicKey%" "%temporaryDirectory%%modName%\Keys\" > nul

mklink /J "%workDrive%%prefixLinkRoot%\" "%githubDirectory%%prefixLinkRoot%\"

mklink /J "%gameDirectory%%modName%\" "%temporaryDirectory%%modName%\"
mklink /J "%serverDirectory%%modName%\" "%temporaryDirectory%%modName%\"

%makePBO% -U -P -D -N "%workDrive%%prefixLinkRoot%\Data" "%temporaryDirectory%%modName%\Addons\Data.pbo"
%signFile% "%privateKey%" "%temporaryDirectory%%modName%\Addons\Data.pbo"

%makePBO% -U -P -D -N "%workDrive%%prefixLinkRoot%\GUI" "%temporaryDirectory%%modName%\Addons\GUI.pbo"
%signFile% "%privateKey%" "%temporaryDirectory%%modName%\Addons\GUI.pbo"

%makePBO% -U -P -D -N "%workDrive%%prefixLinkRoot%\languagecore" "%temporaryDirectory%%modName%\Addons\languagecore.pbo"
%signFile% "%privateKey%" "%temporaryDirectory%%modName%\Addons\languagecore.pbo"

%makePBO% -U -P -D -N "%workDrive%%prefixLinkRoot%\Objects" "%temporaryDirectory%%modName%\Addons\Objects.pbo"
%signFile% "%privateKey%" "%temporaryDirectory%%modName%\Addons\Objects.pbo"

%makePBO% -U -P -D -N "%workDrive%%prefixLinkRoot%\Particles" "%temporaryDirectory%%modName%\Addons\Particles.pbo"
%signFile% "%privateKey%" "%temporaryDirectory%%modName%\Addons\Particles.pbo"

%makePBO% -U -P -D -N "%workDrive%%prefixLinkRoot%\Scripts" "%temporaryDirectory%%modName%\Addons\Scripts.pbo"
%signFile% "%privateKey%" "%temporaryDirectory%%modName%\Addons\Scripts.pbo"

%makePBO% -U -P -D -N "%workDrive%%prefixLinkRoot%\Sounds" "%temporaryDirectory%%modName%\Addons\Sounds.pbo"
%signFile% "%privateKey%" "%temporaryDirectory%%modName%\Addons\Sounds.pbo"

%makePBO% -U -P -D -N "%workDrive%%prefixLinkRoot%\Vehicles\Air" "%temporaryDirectory%%modName%\Addons\Air.pbo"
%signFile% "%privateKey%" "%temporaryDirectory%%modName%\Addons\Air.pbo"

%makePBO% -U -P -D -N "%workDrive%%prefixLinkRoot%\Vehicles\Ground" "%temporaryDirectory%%modName%\Addons\Ground.pbo"
%signFile% "%privateKey%" "%temporaryDirectory%%modName%\Addons\Ground.pbo"

%makePBO% -U -P -D -N "%workDrive%%prefixLinkRoot%\Vehicles\Tracked" "%temporaryDirectory%%modName%\Addons\Tracked.pbo"
%signFile% "%privateKey%" "%temporaryDirectory%%modName%\Addons\Tracked.pbo"

endlocal

cd %batchFileDirectory%