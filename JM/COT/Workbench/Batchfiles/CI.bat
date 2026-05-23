@echo off
setlocal

REM //////////////////// CONFIGURATION START ////////////////////

REM PLEASE REMEMBER when adding mods here to also add them to project.cfg, CI1.bat and CI0_EnumSpecialPaths.bat!

set REQUIRED=Main
set OPTIONAL=

REM ///////////////////// CONFIGURATION END /////////////////////

REM IMPORTANT: CFG parsing WITHOUT enabledelayedexpansion!

if exist "%~dp0..\project.cfg.bat" del "%~dp0..\project.cfg.bat"

for /f "usebackq delims=" %%a in ( "%~dp0..\project.cfg" ) do (
	echo set "%%a">>"%~dp0..\project.cfg.bat"
)

call "%~dp0..\project.cfg.bat"

if exist "%~dp0..\user.cfg.bat" del "..\user.cfg.bat"

for /f "usebackq delims=" %%a in ( "%~dp0..\user.cfg" ) do (
	echo set "%%a">>"%~dp0..\user.cfg.bat"
)

call "%~dp0..\user.cfg.bat"

setlocal enabledelayedexpansion

set skipServerMods=1
set skipPboCleanup=0
set singleMod=0
set skipEnumSpecialPaths=0
set "modsToBuild=%REQUIRED%"
set killGame=0
set killWorkbench=0
set "modsToDeploy="
set "deployableModsToBuild="

for /f "tokens=1,2 delims== usebackq" %%f in ("%~dp0..\deploy.cfg") do (
	set modPrefix=%%f
	if /I "!modPrefix:~0,1!" NEQ "#" if %%g NEQ 0 (
		if defined modsToDeploy (
			set "modsToDeploy=!modsToDeploy! %%f"
			if %%f NEQ Bundle set "deployableModsToBuild=!deployableModsToBuild! %%f"
		) else (
			set "modsToDeploy=%%f"
			if %%f NEQ Bundle set "deployableModsToBuild=%%f"
		)
	)
)

:OPTIONS
REM parse command line options
if "%~1"=="" goto MAIN
if /I "%~1"=="buildServerMods" (
	echo Command line option buildServerMods found - building server mods
	set skipServerMods=0
) else if /I "%~1"=="rebuildAll" (
	REM handled by CI1.bat
) else if /I "%~1"=="skipPboCleanup" (
	echo Command line option skipPboCleanup found - not removing orphaned PBOs
	set skipPboCleanup=1
) else if /I "%~1"=="singleMod" (
	echo Command line option singleMod found - putting all built PBOs in DayZ-Expansion\addons
	set singleMod=1
) else if /I "%~1"=="noIgnore" (
	REM handled by CI1.bat
) else if /I "%~1"=="skipEnumSpecialPaths" (
	echo Command line option skipEnumSpecialPaths found - skipping check for changed "special" file paths
	set skipEnumSpecialPaths=1
) else if /I "%~1"=="useMikerosTools" (
	REM handled by CI1.bat
) else if /I "%~1"=="killGame" (
	echo Command line option killGame found - will close all instances of game ^(client/server^) if running
	set killGame=1
) else if /I "%~1"=="killWorkbench" (
	echo Command line option killWorkbench found - will close all instances of Workbench if running
	set killWorkbench=1
) else if /I "%~1"=="buildDeployableOnly" (
	REM echo Command line option buildDeployableOnly found - will only build mods selected for deployment in deploy.cfg
	REM set "modsToBuild=%deployableModsToBuild%"
) else (
	if "%modsToBuild%"=="%REQUIRED%" (
		set "modsToBuild=%1"
	) else (
		set "modsToBuild=%modsToBuild% %1"
	)
)
shift /1
goto OPTIONS

:MAIN

if %killGame%==1 CALL "%~dp0KillGame.bat"
if %killWorkbench%==1 taskkill /F /IM workbenchApp.exe /T

set "modsToCheck=%modsToBuild%"
echo Mods to build: %modsToBuild%
if "%modsToBuild%" NEQ "%REQUIRED%" (
	echo Enabling implicit skipPboCleanup
	set skipPboCleanup=1
)

set success=1

pushd "%workDrive%%prefixLinkRoot%\"

echo %date% %time% Getting config paths...
dir /B /S config.cpp > "%workDrive%Temp\%PrefixLinkRoot%-ALL-configpaths-unfiltered.list"
echo %date% %time% ...got config paths

if %skipEnumSpecialPaths% NEQ 1 (
	call "%~dp0CI0_EnumSpecialPaths.bat" ALL
)

popd

pushd "%~dp0"

set "singleModName=%ModName%"
set "bundleModName="
set "serverBundleModName="

if exist *.failure del *.failure
if exist *.tmp del *.tmp
if exist *.success del *.success
if exist "%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log" del "%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
if not exist "%workDrive%%prefixLinkRoot%\Workbench\Logs" mkdir "%workDrive%%prefixLinkRoot%\Workbench\Logs"

REM BUNDLE PBOS
if defined bundleModName (
	call "%~dp0CI0_SetupFolders.bat" "%bundleModName%">>"%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
	if errorlevel 1 (
		set success=0
		goto end
	)
)
if %skipServerMods% NEQ 1 if defined serverBundleModName (
	REM SERVER BUNDLE PBOS
	call "%~dp0CI0_SetupFolders.bat" "%serverBundleModName%">>"%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
	if errorlevel 1 (
		set success=0
		goto end
	)
)

if %singleMod%==1 (
	REM SINGLEMOD PBOS
	call "%~dp0CI0_SetupFolders.bat" "%singleModName%" forceSingleModPboNamesRefresh>>"%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
	if errorlevel 1 (
		set success=0
		goto end
	)
)

REM Complete stringtable by setting missing translations to english text and splitting to the various mods
if exist "%~dp0complete_stringtable.exe" "%~dp0complete_stringtable.exe"

REM Build + pack mods
for %%a in (%modsToBuild%) do (
	if not exist CI_%%a.bat (
		echo ERROR: %cd%CI_%%a.bat does not exist.>>"%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
		goto end
	)
	echo !date! !time! CI_%%a.bat is starting
	echo %time% > %%a.tmp
	start "%%a" CI_%%a.bat skipEnumPaths exitCmd %*
)
if "%modsToBuild%"=="%REQUIRED%" (
	for %%a in (%OPTIONAL%) do (
		if exist CI_%%a.bat (
			set "modsToCheck=%modsToCheck% %%a"
			echo !date! !time! CI_%%a.bat is starting
			echo %time% > %%a.tmp
			start "%%a" CI_%%a.bat skipEnumPaths exitCmd %*
		)
	)
)

:loop
echo Waiting 1 seconds
ping -n 2 127.0.0.1 >NUL
set running=0
for %%a in (%modsToCheck%) do (
	if exist %%a.tmp (
		set /a running+=1
		echo CI_%%a.bat is running
	)
)
if %running%==0 goto end
goto loop

:end
if %skipPboCleanup% NEQ 1 (
	if defined bundleModName (
		REM Cleanup Bundle BPOs
		call "%~dp0CI0_DeleteOrphaned.bat" "%bundleModName%" "%bundleModName%"
		if errorlevel 1 set success=0
	)

	if %skipServerMods% NEQ 1 if defined serverBundleModName (
		REM Cleanup Server Bundle BPOs
		call "%~dp0CI0_DeleteOrphaned.bat" "%serverBundleModName%" "%serverBundleModName%"
		if errorlevel 1 set success=0
	)

	REM Cleanup singlemod PBOs
	if %singleMod%==1 (
		call "%~dp0CI0_DeleteOrphaned.bat" "%singleModName%" "%singleModName%"
		if errorlevel 1 set success=0
	)
)
if defined bundleModName call "%~dp0CI_MakeLowercase.bat" "%modBuildDirectory%%bundleModName%"
if %skipServerMods% NEQ 1 if defined serverBundleModName call "%~dp0CI_MakeLowercase.bat" "%modBuildDirectory%%serverBundleModName%"
for %%a in (%modsToBuild%) do (
	if exist %%a.failure (
		set success=0
	)
)
if "!success!" NEQ "1" (
	echo /////////////////////////////////////////////////////////////>>"%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
	echo %date% %time% Something went wrong, check logs. Failed builds:>>"%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
	for %%a in (%modsToCheck%) do (
		if exist %%a.failure echo %%a>>"%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
	)
	echo /////////////////////////////////////////////////////////////>>"%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
) else (
	for %%a in (%modsToBuild%) do (
		if not exist %%a.success (
			set success=0
			echo %date% %time% ERROR: Building %%a did not finish successfully.>>"%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
		)
	)
)
if "!success!"=="1" echo %date% %time% Successfully packaged all mods.>>"%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
type "%workDrive%%prefixLinkRoot%\Workbench\Logs\CI.log"
if "!success!" NEQ "1" goto abort

REM Prepare deploy
for %%n in (experimental internal release) do (
	echo Preparing %%n deployment...
	if %skipServerMods% NEQ 1 (
		type "%workDrive%%prefixLinkRoot%\Workbench\11.txt">"%workDrive%%prefixLinkRoot%\Workbench\%%n"
	) else (
		type "%workDrive%%prefixLinkRoot%\Workbench\10.txt">"%workDrive%%prefixLinkRoot%\Workbench\%%n"
	)


	for %%f in (%modsToDeploy%) do (
			set "modName=!%%fModName!"

			REM Omit @
			set "modName=!modName:~1!"

			echo Preparing "!modName!" deployment...

			echo !modName!>>"%workDrive%%prefixLinkRoot%\Workbench\%%n"
			echo !%%~nfMod_%%n_SteamId!>>"%workDrive%%prefixLinkRoot%\Workbench\%%n"
			if %skipServerMods% NEQ 1 (
				echo !%%~nfMod_Server_%%n_SteamId!>>"%workDrive%%prefixLinkRoot%\Workbench\%%n"
			)
			if "%%n"=="internal" (
				type "%workDrive%%prefixLinkRoot%\Workbench\Steam_PublishedFileVisibilityPrivate.txt">>"%workDrive%%prefixLinkRoot%\Workbench\%%n"
			) else (
				type "%workDrive%%prefixLinkRoot%\Workbench\Steam_PublishedFileVisibilityPublic.txt">>"%workDrive%%prefixLinkRoot%\Workbench\%%n"
			)
	)
)

popd
endlocal
exit /B 0

:abort
popd
endlocal
REM Has to be the last line so it sets the exitcode
exit /B 1