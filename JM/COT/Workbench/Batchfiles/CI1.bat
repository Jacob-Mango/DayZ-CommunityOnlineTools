@echo off
setlocal

REM Call this as CI1.bat <modPrefix> [buildServerMods] [rebuildAll] [skipPboCleanup] [singleMod] [noIgnore]
REM If buildServerMods is given, will build server mods
REM If rebuildAll is given, will rebuild all PBOs, not only changed ones
REM If skipPboCleanup is given when not using rebuildAll, will not remove orphaned PBOs that are no longer part of the build (debug option, don't use in production!)
REM If singleMod is given, will put all built PBOs in DayZ-Expansion\addons (meant to be used ONLY for local builds!)
REM If noIgnore is given, build all PBOs. Presence of IGNORE file will not skip PBO (debug option, don't use in production!)
REM E.g.:
REM       CI1.bat Main

set modPrefix=%1
shift /1
if "%modPrefix%"=="" set modPrefix=Main

REM DO NOT CHANGE THESE!
set skipServerMods=1
set skipUnchanged=1
set skipPboCleanup=0
set singleMod=0
set noIgnore=0
set skipEnumConfigPaths=0
set skipEnumSpecialPaths=0
set useMikerosTools=0

:OPTIONS
REM parse command line options
if "%~1"=="" goto MAIN
if /I "%~1"=="buildServerMods" (
	echo Command line option buildServerMods found - building server mods
	set skipServerMods=0
)
if /I "%~1"=="rebuildAll" (
	echo Command line option rebuildAll found - rebuilding all PBOs
	set skipUnchanged=0
)
if /I "%~1"=="skipPboCleanup" (
	echo Command line option skipPboCleanup found - not removing orphaned PBOs
	set skipPboCleanup=1
)
if /I "%~1"=="singleMod" (
	echo Command line option singleMod found - putting all built PBOs in DayZ-Expansion\addons
	set singleMod=1
)
if /I "%~1"=="noIgnore" (
	echo Command line option noIgnore found - building all PBOs without exceptions
	set noIgnore=1
)
if /I "%~1"=="skipEnumPaths" (
	echo Command line option skipEnumPaths found - skip enumerating config/special paths
	set skipEnumConfigPaths=1
	set skipEnumSpecialPaths=1
)
if /I "%~1"=="useMikerosTools" (
	echo Command line option useMikerosTools found - using Mikeros tools ^(if installed^)
	set useMikerosTools=1
)
shift /1
goto OPTIONS

:MAIN
if exist "%~dp0%modPrefix%.failure" del "%~dp0%modPrefix%.failure"
if exist "%~dp0%modPrefix%.success" del "%~dp0%modPrefix%.success"

cd /D "%~dp0"

echo %time% > %modPrefix%.tmp

set /a failed=0

if exist ../project.cfg (
	echo Found the project.cfg
) else (
	echo Failed to find the project.cfg file, exitting.
	set /a failed=1
)

if exist ../user.cfg (
	echo Found the user.cfg
) else (
	echo Failed to find the user.cfg file, exitting.
	set /a failed=1
)

if %failed%==1 goto ABORT

set "githubDirectory=%cd%\"
set "workbenchDataDirectory=%githubDirectory%Workbench\"
set "toolsDirectory=%workbenchDataDirectory%Tools\"

set workDrive=
set mainModName=
set modName=
set clientModName=
set serverModName=
set singleModName=
set modBuildDirectory=
set prefixLinkRoot=
set keyDirectory=
set keyName=

REM IMPORTANT: CFG parsing WITHOUT enabledelayedexpansion!

if exist "..\project.cfg.%modPrefix%.bat" del "..\project.cfg.%modPrefix%.bat"

for /f "usebackq delims=" %%a in ( ../project.cfg ) do (
	echo set %%a>>"..\project.cfg.%modPrefix%.bat"
)

call "..\project.cfg.%modPrefix%.bat"

if exist "..\user.cfg.%modPrefix%.bat" del "..\user.cfg.%modPrefix%.bat"

for /f "usebackq delims=" %%a in ( ../user.cfg ) do (
	echo set %%a>>"..\user.cfg.%modPrefix%.bat"
)

call "..\user.cfg.%modPrefix%.bat"

setlocal enableextensions enabledelayedexpansion

if skipEnumConfigPaths==1 if not exist "%workDrive%Temp\%PrefixLinkRoot%-ALL-configpaths-unfiltered.list" set skipEnumConfigPaths=0
if skipEnumSpecialPaths==1 if not exist "%workDrive%Temp\%PrefixLinkRoot%-ALL-specialpaths.list" set skipEnumSpecialPaths=0

if %singleMod%==1 set "singleModName=%ModName%"
set "bundleModName="
set "serverBundleModName="

if "!%modPrefix%ModName!" NEQ "" (
	set "bundleModName=%ModName%-Bundle"
	set "modName=!%modPrefix%ModName!"
	set "serverBundleModName=%bundleModName%-Server"
)
set "clientModName=%modName%"
set "serverModName=%modName%-Server"

echo KeyDirectory is: "%keyDirectory%"
if "%keyDirectory%"=="" (
	set /a failed=1
	echo KeyDirectory parameter was not set in the project.cfg
)

echo KeyName is: "%keyName%"
if "%keyName%"=="" (
	set /a failed=1
	echo KeyName parameter was not set in the project.cfg
)

echo %modPrefix%ModName is: "%modName%"
if "%modName%"=="" (
	set /a failed=1
	echo %modPrefix%ModName parameter was not set in the project.cfg
)

echo WorkDrive is: "%workDrive%"
if "%workDrive%"=="" (
	set /a failed=1
	echo WorkDrive parameter was not set in the project.cfg
)

echo ModBuildDirectory is: "%modBuildDirectory%"
if "%modBuildDirectory%"=="" (
	set /a failed=1
	echo ModBuildDirectory parameter was not set in the project.cfg
)

echo PrefixLinkRoot is: "%prefixLinkRoot%"
if "%prefixLinkRoot%"=="" (
	set /a failed=1
	echo PrefixLinkRoot parameter was not set in the project.cfg
)

if %failed%==1 goto ABORT

if %useMikerosTools% NEQ 1 GOTO dayztools

echo Searching for Mikero Tools...
for /F "Tokens=2* skip=2" %%A In ('REG QUERY "HKCU\SOFTWARE\Mikero\depbo" /v "path" 2^>nul') do (set "_MIKEDLL=%%~B")
if not defined _MIKEDLL (
	for /F "Tokens=2* skip=2" %%A In ('REG QUERY "HKLM\SOFTWARE\Mikero\depbo" /v "path" 2^>nul') do (set "_MIKEDLL=%%~B")
	if not defined _MIKEDLL (
		echo Mikero Tools was not set in the registry path, trying default.
		set "_MIKEDLL=C:\Program Files (x86)\Mikero\DePboTools"
	) else (
		echo Found. "%_MIKEDLL%"
	)
) else (
	echo Found.
)

IF NOT EXIST "%_MIKEDLL%\bin\Rapify.exe" (
	echo "%_MIKEDLL%\bin\Rapify.exe" not found
	goto ABORT
)

IF NOT EXIST "%_MIKEDLL%\bin\MakePbo.exe" (
	echo "%_MIKEDLL%\bin\MakePbo.exe" not found
	goto ABORT
)

:dayztools
echo Searching for DayZ Tools...
for /F "Tokens=2* skip=2" %%A In ('REG QUERY "HKLM\Software\Bohemia Interactive\DayZ\Tools" /v "Path" 2^>nul') do (set "_DAYZTOOLSPATH=%%~B")
if not defined _DAYZTOOLSPATH (
	for /F "Tokens=2* skip=2" %%A In ('REG QUERY "HKCU\Software\Bohemia Interactive\Dayz Tools" /v "Path" 2^>nul') do (set "_DAYZTOOLSPATH=%%~B\Bin")
)
if not defined _DAYZTOOLSPATH (
	echo DayZ Tools was not set in the registry path, trying default.
	set "_DAYZTOOLSPATH=C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools"
) else (
	echo Found. "%_DAYZTOOLSPATH%"
)

IF NOT EXIST "%_DAYZTOOLSPATH%\CfgConvert\CfgConvert.exe" (
	echo "%_DAYZTOOLSPATH%\CfgConvert\CfgConvert.exe" not found
	goto ABORT
)

IF NOT EXIST "%_DAYZTOOLSPATH%\binarize\binarize.exe" (
	echo "%_DAYZTOOLSPATH%\binarize\binarize.exe" not found
	goto ABORT
)

IF NOT EXIST "%_DAYZTOOLSPATH%\DsUtils\DSSignFile.exe" (
	echo "%_DAYZTOOLSPATH%\DsUtils\DSSignFile.exe" not found
	goto ABORT
)


if %skipUnchanged%==1 GOTO SETUPFOLDERS

REM CLEANING UP JUST IN CASE

REM CLIENT PBOS

IF exist "%workDrive%Temp\%modName%\addons\" (
	echo Removing folder "%workDrive%Temp\%modName%\addons\"
	del /f /s /q "%workDrive%Temp\%modName%\addons\" > nul
	rmdir /s /q "%workDrive%Temp\%modName%\addons\"
)

IF exist "%workDrive%Temp\%modName%\" (
	echo Removing folder "%workDrive%Temp\%modName%\"
	del /f /s /q "%workDrive%Temp\%modName%\" > nul
	rmdir /s /q "%workDrive%Temp\%modName%\"
)

IF exist "%modBuildDirectory%%modName%\addons" (
	echo Removing folder "%modBuildDirectory%%modName%\addons\"
	del /f /s /q "%modBuildDirectory%%modName%\addons\" > nul
	rmdir /s /q "%modBuildDirectory%%modName%\addons\"
)

IF exist "%modBuildDirectory%%modName%\" (
	echo Removing folder "%modBuildDirectory%%modName%\"
	rmdir /S /Q "%modBuildDirectory%%modName%\"
)

REM SERVER PBOS
if %skipServerMods%==1 goto SETUPFOLDERS

IF exist "%workDrive%Temp\%serverModName%\addons\" (
	echo Removing folder "%workDrive%Temp\%serverModName%\addons\"
	del /f /s /q "%workDrive%Temp\%serverModName%\addons\" > nul
	rmdir /s /q "%workDrive%Temp\%serverModName%\addons\"
)

IF exist "%workDrive%Temp\%serverModName%\" (
	echo Removing folder "%workDrive%Temp\%serverModName%\"
	del /f /s /q "%workDrive%Temp\%serverModName%\" > nul
	rmdir /s /q "%workDrive%Temp\%serverModName%\"
)

IF exist "%modBuildDirectory%%serverModName%\addons" (
	echo Removing folder "%modBuildDirectory%%serverModName%\addons\"
	del /f /s /q "%modBuildDirectory%%serverModName%\addons\" > nul
	rmdir /s /q "%modBuildDirectory%%serverModName%\addons\"
)

IF exist "%modBuildDirectory%%serverModName%\" (
	echo Removing folder "%modBuildDirectory%%serverModName%\"
	rmdir /S /Q "%modBuildDirectory%%serverModName%\"
)

:SETUPFOLDERS

REM CLIENT PBOS
call "%~dp0CI0_SetupFolders.bat" "%modName%"
if errorlevel 1 goto ABORT

REM SERVER PBOS
if %skipServerMods% NEQ 1 (
	call "%~dp0CI0_SetupFolders.bat" "%serverModName%"
	if errorlevel 1 goto ABORT
)


if %skipServerMods% NEQ 1 (
	echo Packaging "%modName%" and "%serverModName%" PBOs
) else (
	echo Packaging "%modName%" PBOs
)

cd /D "%workDrive%%prefixLinkRoot%\"

set /a configcpps=0

IF EXIST "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths-unfiltered.list" (
	move /Y "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths-unfiltered.list" "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths-unfiltered-previous.list"
)

if !skipEnumConfigPaths!==0 (
	echo %date% %time% Getting config paths...
	dir /B /S config.cpp > "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths-unfiltered.list"
	echo %date% %time% ...got config paths
) else (
	copy /Y "%workDrive%Temp\%PrefixLinkRoot%-ALL-configpaths-unfiltered.list" "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths-unfiltered.list"
)

REM Check if config paths changed from previous run
IF EXIST "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths-unfiltered-previous.list" (
	fc "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths-unfiltered.list" "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths-unfiltered-previous.list" > NUL && (
		REM config paths unchanged from previous run, check "special" file paths

		IF EXIST "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-specialpaths.list" (
			move /Y "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-specialpaths.list" "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-specialpaths-previous.list"
		)

		if !skipEnumSpecialPaths!==0 (
			call "%~dp0CI0_EnumSpecialPaths.bat" "%modPrefix%"
		) else (
			copy /Y "%workDrive%Temp\%PrefixLinkRoot%-ALL-specialpaths.list" "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-specialpaths.list"
		)

		fc "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-specialpaths.list" "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-specialpaths-previous.list" > NUL && (
			IF EXIST "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths.list" (
				echo !date! !time! Config and "special" file paths unchanged, skipping check
				GOTO build
			)
		)
	)
)

IF EXIST "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths.list" del /F "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths.list"

echo %date% %time% Checking config paths...
for /F "usebackq delims=" %%D in ( "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths-unfiltered.list" ) do (
	call :CHECKCONFIGPATH "%%~D"
)
echo %date% %time% ...checked config paths

IF !configcpps!==0 (
	echo /////////////////////////////////////////////////////////////
	echo %date% %time% ERROR: Found no config.cpp files for %modPrefix% in "%workDrive%%prefixLinkRoot%"
	echo /////////////////////////////////////////////////////////////
	GOTO ABORT
)

echo %date% %time% Found !configcpps! config.cpp files for %modPrefix% in "%workDrive%%prefixLinkRoot%"

GOTO BUILD

:CHECKCONFIGPATH path
if "%modPrefix%"=="Main" (
	REM Do NOT build if a FILE (not a folder) with any of the following names exists next to config.cpp
) else (
	REM Build if a FILE with the name <modPrefix> exists next to config.cpp, but NOT if a FOLDER with that name exists
	IF NOT EXIST "%~dp1%modPrefix%_" (
		IF NOT EXIST "%~dp1%modPrefix%" exit /B
		IF EXIST "%~dp1%modPrefix%\*" exit /B
	)
)
IF EXIST "%~dp1IGNORE" IF %noIgnore%==0 (
	echo PBO set to ignore, skipping %1
	exit /B
)

IF EXIST "%~dp1..\config.cpp"                      IF NOT EXIST "%~dp1..\%PrefixLinkRoot%" exit /B
IF EXIST "%~dp1..\..\config.cpp"                   IF NOT EXIST "%~dp1..\..\%PrefixLinkRoot%" exit /B
IF EXIST "%~dp1..\..\..\config.cpp"                IF NOT EXIST "%~dp1..\..\..\%PrefixLinkRoot%" exit /B
IF EXIST "%~dp1..\..\..\..\config.cpp"             IF NOT EXIST "%~dp1..\..\..\..\%PrefixLinkRoot%" exit /B
IF EXIST "%~dp1..\..\..\..\..\config.cpp"          IF NOT EXIST "%~dp1..\..\..\..\..\%PrefixLinkRoot%" exit /B
IF EXIST "%~dp1..\..\..\..\..\..\config.cpp"       IF NOT EXIST "%~dp1..\..\..\..\..\..\%PrefixLinkRoot%" exit /B
IF EXIST "%~dp1..\..\..\..\..\..\..\config.cpp"    IF NOT EXIST "%~dp1..\..\..\..\..\..\..\%PrefixLinkRoot%" exit /B
IF EXIST "%~dp1..\..\..\..\..\..\..\..\config.cpp" IF NOT EXIST "%~dp1..\..\..\..\..\..\..\..\%PrefixLinkRoot%" exit /B

rem echo No parent config.cpp found, building pbo %1
set /a configcpps+=1
echo "%~1">>"%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths.list"
exit /B

:BUILD

IF NOT EXIST "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths.list" (
	echo /////////////////////////////////////////////////////////////
	echo %date% %time% ERROR: "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths.list" could not be created or was deleted behind our back
	echo /////////////////////////////////////////////////////////////
	GOTO ABORT
)

if /i "%modPrefix%"=="Core" (
	REM Update version define in 0_Core_Preload if needed
	set "version=#define EXPANSION"
	for /f "usebackq tokens=1,2,3 delims=." %%f in (`call "%~dp0EchoVersion.bat"`) do (
		set "version=!version!_%%f_%%g_%%h"
	)
	findstr /X /C:"!version!" "%workDrive%%PrefixLinkRoot%\0_Core_Preload\Common\DayZExpansion_Core_Defines.c" || (
		findstr /V /R /C:"#define EXPANSION_[0-9]*_[0-9]*_[0-9]*" "%workDrive%%PrefixLinkRoot%\0_Core_Preload\Common\DayZExpansion_Core_Defines.c" > "%workDrive%%PrefixLinkRoot%\0_Core_Preload\Common\DayZExpansion_Core_Defines.c~"
		move /Y "%workDrive%%PrefixLinkRoot%\0_Core_Preload\Common\DayZExpansion_Core_Defines.c~" "%workDrive%%PrefixLinkRoot%\0_Core_Preload\Common\DayZExpansion_Core_Defines.c"
		echo !version!>>"%workDrive%%PrefixLinkRoot%\0_Core_Preload\Common\DayZExpansion_Core_Defines.c"
	)
)

if not exist "%workDrive%DZ\vehicles\proxies" mkdir "%workDrive%DZ\vehicles\proxies"
if not exist "%workDrive%DZ\vehicles\proxies\bus_cargo.p3d" echo.>"%workDrive%DZ\vehicles\proxies\bus_cargo.p3d"
if not exist "%workDrive%DZ\vehicles\proxies\bus_driver.p3d" echo.>"%workDrive%DZ\vehicles\proxies\bus_driver.p3d"

for /F "usebackq delims=" %%D in ("%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-configpaths.list") do (

	set "folderToBuild=%%~pD"

	set "pboName=%%~pD"
	IF "!pboName:~-1!"=="\" SET "pboName=!pboName:~0,-1!"
	set "pboName=!pboName:\%prefixLinkRoot%\=!"
	set "pboName=!pboName:\=_!"

	set "prefixName=%%~pD"
	IF "!prefixName:~-1!"=="\" SET "prefixName=!prefixName:~0,-1!"
	set "prefixName=!prefixName:\%prefixLinkRoot%\=!"
	set "prefixName=%prefixLinkRoot%\!prefixName!"

	set currentFolder=
	for %%I in ( !folderToBuild!\config.cpp\.. ) do (
		set "currentFolder=%%~nxI"
	)

	set filesChanged=0
	CALL :BINARIZE "!prefixName!" "%modName%" "%serverModName%"
	IF ERRORLEVEL 2 GOTO ABORT
	IF ERRORLEVEL 1 set filesChanged=1

	CALL :PACK !currentFolder! "!pboName!" "!prefixName!" "%modName%" !filesChanged!
	IF ERRORLEVEL 1 GOTO ABORT

	REM Check if other build process failed
	IF EXIST "%~dp0*.failure" (
		echo !date! !time! Aborting because other build^(s^) failed before:
		dir /B "%~dp0*.failure"
		goto EXIT
	)

	if %skipServerMods% NEQ 1 CALL :PACK !currentFolder! "!pboName!" "!prefixName!" "%serverModName%" !filesChanged!
	IF ERRORLEVEL 1 GOTO ABORT

	REM Check if other build process failed
	IF EXIST "%~dp0*.failure" (
		echo !date! !time! Aborting because other build^(s^) failed before:
		dir /B "%~dp0*.failure"
		goto EXIT
	)

	REM Create dummy file so we can later remove orphaned PBOs that are no longer part of the build (if any)
	echo.>"%workDrive%Temp\PboNames\%modName%\!pboName!.pbo"
	if %singleMod%==1 if /I "%singleModName%" NEQ "%modName%" (
		echo.>"%workDrive%Temp\PboNames\%singleModName%\!pboName!.pbo"
	)
	if /I "%modName%" NEQ "%LicensedModName%" (
		if defined bundleModName echo.>"%workDrive%Temp\PboNames\%bundleModName%\!pboName!.pbo"
		if defined serverBundleModName echo.>"%workDrive%Temp\PboNames\%serverBundleModName%\!pboName!.pbo"
	)

	if exist "%%~dpD..\CHANGES" copy /Y "%%~dpD..\CHANGES" "%modBuildDirectory%%modName%\CHANGES"
	if exist "%%~dpD..\LICENSE" copy /Y "%%~dpD..\LICENSE" "%modBuildDirectory%%modName%\LICENSE"
	if exist "%%~dpD..\NOTICE.txt" copy /Y "%%~dpD..\NOTICE.txt" "%modBuildDirectory%%modName%\NOTICE.txt"
	if exist "%%~dpD..\CREDITS.txt" copy /Y "%%~dpD..\CREDITS.txt" "%modBuildDirectory%%modName%\CREDITS.txt"

)

if %skipPboCleanup% NEQ 1 (
	if %singleMod%==1 if /I "%modName%"=="%singleModName%" GOTO END
	call "%~dp0CI0_DeleteOrphaned.bat" "%modName%" "%modName%"
	IF ERRORLEVEL 1 GOTO ABORT
	if %skipServerMods% NEQ 1 (
		call "%~dp0CI0_DeleteOrphaned.bat" "%modName%" "%serverModName%"
		IF ERRORLEVEL 1 GOTO ABORT
	)
)

:END
call "%~dp0CI_MakeLowercase.bat" "%modBuildDirectory%%modName%"
if %skipServerMods% NEQ 1 call "%~dp0CI_MakeLowercase.bat" "%modBuildDirectory%%serverModName%"

echo %time% > "%~dp0%modPrefix%.success"
echo %date% %time% Successfully packaged %modPrefix%.
goto EXIT

:BINARIZE prefixName modName serverModName

setlocal enableextensions enabledelayedexpansion

set "prefixName=%~1"
set "modName=%~2"
set "serverModName=%~3"
set toUpdate=0

IF "%prefixName%"=="%prefixLinkRoot%\languagecore\Core" (
	REM languagecore\Core changed from a filename to a dirname, need to check if we need to do some cleanup
	IF EXIST "%workDrive%Temp\%prefixName%" IF NOT EXIST "%workDrive%Temp\%prefixName%\*" (
		echo Removing file "%workDrive%Temp\%prefixName%"
		del "%workDrive%Temp\%prefixName%"
	)
)

set changes=0
echo %date% %time% Mirroring "%workDrive%%prefixName%" to "%workDrive%Temp\%prefixName%"...
robocopy "%workDrive%%prefixName%" "%workDrive%Temp\%prefixName%" /MIR /XF *.bak /XD *.bak /XF *.png /XF *.psd /XF *.tif /XF *.log /XD source /XF *.tga /XF *.bat /XF *.cmd /XF *.pbo /XF *.bisign /XF *.blend? /NDL /NP /NJH /NJS
IF ERRORLEVEL 8 (
	REM Copy error

	echo /////////////////////////////////////////////////////////////
	echo %date% %time% RoboCopy could not mirror "%workDrive%%prefixName%" to "%workDrive%Temp\%prefixName%"
	echo /////////////////////////////////////////////////////////////

	endlocal
	exit /B 2
)
IF ERRORLEVEL 1 set changes=1
REM When (re-)creating a missing PBO, we need to create config.bin
IF NOT EXIST "%modBuildDirectory%%modName%\addons\%pboName%.pbo" set changes=1
if %skipServerMods% NEQ 1 IF NOT EXIST "%modBuildDirectory%%serverModName%\addons\%pboName%.pbo" set changes=1
if %changes%==1 (
	REM Files changed/copied
	echo %date% %time% Changes in "%workDrive%Temp\%prefixName%"

	if %skipServerMods% NEQ 1 (
		set toUpdate="%clientModName%" "%serverModName%"
	) else (
		set toUpdate="%clientModName%"
	)
)
IF "!toUpdate!"=="0" (
	robocopy "%workDrive%Temp\%prefixName%" "%workDrive%Temp\BinarizeCache\%clientModName%\%prefixName%" *.p3d *.wrp /L /S /XC /XN /XO > nul 2>&1 || (
		echo %date% %time% Binarized client files missing, will regenerate them

		set toUpdate="%clientModName%"
	)
	IF %skipServerMods% NEQ 1 (
		robocopy "%workDrive%Temp\%prefixName%" "%workDrive%Temp\BinarizeCache\%serverModName%\%prefixName%" *.p3d *.wrp /L /S /XC /XN /XO > nul 2>&1 || (
			echo !date! !time! Binarized server files missing, will regenerate them

			if "!toUpdate!"=="0" (
				set toUpdate="%serverModName%"
			) else (
				set toUpdate="!toUpdate!" "%serverModName%"
			)
		)
	)
)
IF "!toUpdate!" NEQ "0" (
	set failed=0
	REM Rapify
	echo !date! !time! Binarizing "%workDrive%Temp\%prefixName%\config.cpp"
	pushd %workDrive%
	if %useMikerosTools%==1 (
		"%_MIKEDLL%\bin\Rapify.exe" -Pe "%workDrive%Temp\%prefixName%\config.cpp" "%workDrive%Temp\%prefixName%\config.bin" 2>&1
	) else (
		for /R "%workDrive%Temp\%prefixName%" %%a in (*.cpp) do (
			"%_DAYZTOOLSPATH%\CfgConvert\CfgConvert.exe" -bin -dst "%%~dpna.bin" "%%~a" 2>&1
		)
	)
	IF ERRORLEVEL 1 set failed=1
	IF NOT EXIST "%workDrive%Temp\%prefixName%\config.bin" set failed=1
	REM Delete config.cpp so Binarize doesn't choke on it (it'll use config.bin just fine)
	REM This is also necessary to make skipUnchanged work correctly
	echo !date! !time! Deleting temp "%workDrive%Temp\%prefixName%\config.cpp"
	del "%workDrive%Temp\%prefixName%\config.cpp"
	popd
	IF !failed!==1 (
		echo /////////////////////////////////////////////////////////////
		echo !date! !time! Rapify failed on "%workDrive%Temp\%prefixName%\config.cpp"
		echo /////////////////////////////////////////////////////////////

		endlocal
		exit /B 2
	)
	for %%a in (%toUpdate%) do (
		IF EXIST "%workDrive%Temp\BinarizeCache\%%~a\%prefixName%" (
			echo !date! !time! Deleting outdated *.p3d and *.wrp in "%workDrive%Temp\BinarizeCache\%%~a\%prefixName%"...
			del /f /s /q "%workDrive%Temp\BinarizeCache\%%~a\%prefixName%\*.p3d"
			del /f /s /q "%workDrive%Temp\BinarizeCache\%%~a\%prefixName%\*.wrp"
		) else (
			mkdir "%workDrive%Temp\BinarizeCache\%%~a\%prefixName%"
		)
		REM Create directory structure only
		robocopy "%workDrive%Temp\%prefixName%" "%workDrive%Temp\BinarizeCache\%%~a\%prefixName%" *.p3d *.wrp /MIR /XF * > nul 2>&1
		REM Binarize p3d and wrp
		pushd %workDrive%
		if "%%~a"=="%clientModName%" (
			echo !date! !time! Binarizing *.p3d and *.wrp from "%workDrive%Temp\%prefixName%" to "%workDrive%Temp\BinarizeCache\%clientModName%\%prefixName%"...

			"%_DAYZTOOLSPATH%\binarize\binarize.exe" -targetBonesInterval=56 -textures=%workDrive%Temp -binPath=%workDrive% "%workDrive%Temp\%prefixName%" "%workDrive%Temp\BinarizeCache\%clientModName%\%prefixName%"
			IF ERRORLEVEL 1 set failed=1
		) else (
			echo !date! !time! Binarizing and shrinking *.p3d and *.wrp from "%workDrive%Temp\%prefixName%" to "%workDrive%Temp\BinarizeCache\%serverModName%\%prefixName%"...

			"%_DAYZTOOLSPATH%\binarize\binarize.exe" -targetBonesInterval=56 -textures=%workDrive%Temp -binPath=%workDrive% -shrinkP3D -exclude=*.wrp "%workDrive%Temp\%prefixName%" "%workDrive%Temp\BinarizeCache\%serverModName%\%prefixName%"
			IF ERRORLEVEL 1 set failed=1
			REM Just copy over any client mod WRPs
			robocopy "%workDrive%Temp\BinarizeCache\%clientModName%\%prefixName%" "%workDrive%Temp\BinarizeCache\%serverModName%\%prefixName%" *.wrp /S /NDL /NP /NJH /NJS
			IF ERRORLEVEL 8 (
				REM Copy error
				set failed=1

				echo /////////////////////////////////////////////////////////////
				echo %date% %time% RoboCopy could not copy *.wrp from "%workDrive%Temp\BinarizeCache\%clientModName%\%prefixName%" to "%workDrive%Temp\BinarizeCache\%serverModName%\%prefixName%"
				echo /////////////////////////////////////////////////////////////
			)
		)
		popd
		IF !failed!==1 (
			echo /////////////////////////////////////////////////////////////
			echo !date! !time! Something went wrong with %prefixName% during binarizing
			echo /////////////////////////////////////////////////////////////

			endlocal
			exit /B 2
		)
	)
	REM Restore config.cpp
	echo !date! !time! Restoring "%workDrive%Temp\%prefixName%\config.cpp"
	copy "%workDrive%%prefixName%\config.cpp" "%workDrive%Temp\%prefixName%\config.cpp"
	endlocal
	exit /B 1
)
endlocal
exit /B 0
REM End :BINARIZE

:PACK currentFolder pboName prefixName modName filesChanged

setlocal enableextensions enabledelayedexpansion

set "currentFolder=%~1"
set "pboName=%~2"
set "prefixName=%~3"
set "modName=%~4"
set filesChanged=%5

IF %filesChanged%==0 IF %skipUnchanged%==1 (
	REM Recover from earlier move failures
	IF EXIST "%workDrive%Temp\%modName%\addons\%pboName%.pbo" (
		set filesChanged=1
		GOTO MOVEPACKED
	)

	REM Check if PBO already exists
	IF EXIST "%modBuildDirectory%%modName%\addons\%pboName%.pbo" (
		IF EXIST "%modBuildDirectory%%modName%\addons\%pboName%.pbo.%keyName%.bisign" (
			REM Nothing to do
			echo !date! !time! No change in "%workDrive%Temp\%prefixName%", nothing to do for "%modBuildDirectory%%modName%\addons\%pboName%.pbo"
		) else (
			set filesChanged=1
			CALL :SIGN "%modName%" "%pboName%"
		)
		goto ENDPACK
	)
	set filesChanged=1
)

set binarizeCache=0
dir /S "%workDrive%Temp\BinarizeCache\%modName%\%prefixName%\*.p3d" > nul 2>&1 && set binarizeCache=1
IF %binarizeCache%==0 (
	dir /S "%workDrive%Temp\BinarizeCache\%modName%\%prefixName%\*.wrp" > nul 2>&1 && set binarizeCache=1
)
IF %binarizeCache%==1 (
	echo !date! !time! Copying any cached binarized *.p3d and *.wrp from "%workDrive%Temp\BinarizeCache\%modName%\%prefixName%" to "%workDrive%Temp\%prefixName%"...
	robocopy "%workDrive%Temp\BinarizeCache\%modName%\%prefixName%" "%workDrive%Temp\%prefixName%" *.p3d *.wrp /S /NDL /NP /NJH /NJS
	IF ERRORLEVEL 8 (
		REM Copy error

		echo /////////////////////////////////////////////////////////////
		echo !date! !time! RoboCopy could not copy *.p3d or *.wrp from "%workDrive%Temp\BinarizeCache\%modName%\%prefixName%" to "%workDrive%Temp\%prefixName%"
		echo /////////////////////////////////////////////////////////////

		endlocal
		exit /B 1
	)
)

set "pboPrefix=!prefixName!"

if exist "%workDrive%%prefixName%\PREFIX.txt" (
	set /p pboPrefix=<"%workDrive%%prefixName%\PREFIX.txt"
) else (
	REM Special case for PBOs that should be loaded before all other mods
	IF "!pboName:~0,2!"=="0_" set "pboPrefix=!pboPrefix:%prefixLinkRoot%\0_=0_%prefixLinkRoot%_!"
)

echo %date% %time% PREFIX: %pboPrefix%

echo %date% %time% Creating "%workDrive%Temp\%modName%\addons\%pboName%.pbo"...
set failed=0
if %useMikerosTools%==1 (
	"%_MIKEDLL%\bin\MakePbo.exe" -P -X="thumbs.db,*.h,*.dep,*.cpp,*.bak,*.png,*.psd,*.tif,*.log,*.pew,*.hpp,source,*.tga,*.bat,*.cmd,*.pbo,*.bisign,*.cfg,*.blend,*.blend1" "-@=%pboPrefix%" "%workDrive%Temp\%prefixName%" "%workDrive%Temp\%modName%\addons\%pboName%.pbo" 2>&1
) else (
	"%_DAYZTOOLSPATH%\PboUtils\FileBank.exe" -exclude "%~dp0..\excludes.lst" -property prefix=%pboPrefix% -dst "%workDrive%Temp\%modName%\addons" "%workDrive%Temp\%prefixName%" 2>&1
	IF NOT ERRORLEVEL 1 move /Y "%workDrive%Temp\%modName%\addons\%currentFolder%.pbo" "%workDrive%Temp\%modName%\addons\%pboName%.pbo"
)
IF ERRORLEVEL 1 (
	set failed=1

	echo /////////////////////////////////////////////////////////////
	echo %date% %time% Something went wrong with packing %pboName%.pbo for "%modName%"
	echo /////////////////////////////////////////////////////////////

	IF EXIST "%workDrive%Temp\%prefixName%\config.cpp" (
		REM So we detect changes again on next build attempt
		echo %date% %time% Deleting temp "%workDrive%Temp\%prefixName%\config.cpp"
		del "%workDrive%Temp\%prefixName%\config.cpp"
	)
)

IF EXIST "%workDrive%Temp\%prefixName%\config.bin" (
	REM This is necessary to make skipping binarization in case of no changes work
	set delcfgbin=1
	if %skipServerMods% NEQ 1 (
		if "%modName%" NEQ "%serverModName%" set delcfgbin=0
	)
	if !delcfgbin!==1 (
		echo %date% %time% Deleting temp "%workDrive%Temp\%prefixName%\config.bin"
		del /S "%workDrive%Temp\%prefixName%\config.bin"
	)
)

if %binarizeCache%==1 (
	REM This is necessary to make skipping binarization in case of no changes work
	echo %date% %time% Restoring any original *.p3d and *.wrp from "%workDrive%%prefixName%" to "%workDrive%Temp\%prefixName%"...
	robocopy "%workDrive%%prefixName%" "%workDrive%Temp\%prefixName%" *.p3d *.wrp /S /NDL /NP /NJH /NJS
)

IF EXIST "%workDrive%%prefixName%\stringtable.csv" (
	REM This is necessary to make skipping packing in case of no changes work
	echo %date% %time% Restoring original stringtable.csv from "%workDrive%%prefixName%" to "%workDrive%Temp\%prefixName%"...
	copy /Y "%workDrive%%prefixName%\stringtable.csv" "%workDrive%Temp\%prefixName%\stringtable.csv"
)

IF %failed%==1 (
	endlocal
	exit /B 1
)

IF NOT EXIST "%workDrive%Temp\%modName%\addons\%pboName%.pbo" (
	echo /////////////////////////////////////////////////////////////
	echo %date% %time% Something went wrong with %pboName%.pbo for "%modName%", "%workDrive%Temp\%modName%\addons\%pboName%.pbo" doesn't exist
	echo /////////////////////////////////////////////////////////////

	endlocal
	exit /B 1
)

:MOVEPACKED
echo Moving "%workDrive%Temp\%modName%\addons\%pboName%.pbo" to "%modBuildDirectory%%modName%\addons\"
move /Y "%workDrive%Temp\%modName%\addons\%pboName%.pbo" "%modBuildDirectory%%modName%\addons\"
IF ERRORLEVEL 1 (
	echo /////////////////////////////////////////////////////////////
	echo %date% %time% Something went wrong with %pboName%.pbo for "%modName%", it could not be moved to "%modBuildDirectory%%modName%\addons\"
	echo /////////////////////////////////////////////////////////////

	endlocal
	exit /B 1
)

CALL :SIGN "%modName%" "%pboName%"

:ENDPACK
set deploy=!filesChanged!
set bundleDeploy=!filesChanged!
if "%modName%"=="%serverModName%" if defined serverBundleModName (
	REM Server bundle
	if /I "%clientModName%" NEQ "%LicensedModName%" (
		if !filesChanged! NEQ 0 if /I "%modBuildDirectory%%serverBundleModName%" NEQ "%modBuildDirectory%%modName%" (
			IF EXIST "%modBuildDirectory%%serverBundleModName%\addons\%pboName%.pbo" (
				del "%modBuildDirectory%%serverBundleModName%\addons\%pboName%.pbo"
				IF EXIST "%modBuildDirectory%%serverBundleModName%\addons\%pboName%.pbo" (
					call :LINKERROR "%pboName%.pbo" "%modName%" "%modBuildDirectory%%serverBundleModName%\addons\"
					REM make sure it's dealt with next run by deleting source file
					del "%modBuildDirectory%%modName%\addons\%pboName%.pbo"
					endlocal
					exit /B 1
				)
			)
			IF EXIST "%modBuildDirectory%%serverBundleModName%\addons\%pboName%.pbo.%keyName%.bisign" (
				del "%modBuildDirectory%%serverBundleModName%\addons\%pboName%.pbo.%keyName%.bisign"
				IF EXIST "%modBuildDirectory%%serverBundleModName%\addons\%pboName%.pbo.%keyName%.bisign" (
					call :LINKERROR "%pboName%.pbo.%keyName%.bisign" "%modName%" "%modBuildDirectory%%serverBundleModName%\addons\"
					REM make sure it's dealt with next run by deleting source file
					del "%modBuildDirectory%%modName%\addons\%pboName%.pbo.%keyName%.bisign"
					endlocal
					exit /B 1
				)
			)
		)
		IF NOT EXIST "%workDrive%%prefixName%\BUNDLEIGNORE" (
			IF NOT EXIST "%modBuildDirectory%%serverBundleModName%\addons\%pboName%.pbo" (
				mklink /H "%modBuildDirectory%%serverBundleModName%\addons\%pboName%.pbo" "%modBuildDirectory%%modName%\addons\%pboName%.pbo"
				set bundleDeploy=1
			)
			IF NOT EXIST "%modBuildDirectory%%serverBundleModName%\addons\%pboName%.pbo.%keyName%.bisign" (
				mklink /H "%modBuildDirectory%%serverBundleModName%\addons\%pboName%.pbo.%keyName%.bisign" "%modBuildDirectory%%modName%\addons\%pboName%.pbo.%keyName%.bisign"
				set bundleDeploy=1
			)
		)
	)
)
if "%modName%" NEQ "%clientModName%" goto EXITPACK
IF %singleMod%==1 (
	REM If singleMod folder exists, just hardlink everything (creating symlinks would be preferable but requires elevated privileges)
	if !filesChanged! NEQ 0 if /I "%singleModName%" NEQ "%modName%" (
		IF EXIST "%modBuildDirectory%%singleModName%\addons\%pboName%.pbo" (
			del "%modBuildDirectory%%singleModName%\addons\%pboName%.pbo"
			IF EXIST "%modBuildDirectory%%singleModName%\addons\%pboName%.pbo" (
				call :LINKERROR "%pboName%.pbo" "%modName%" "%modBuildDirectory%%singleModName%\addons\"
				REM make sure it's dealt with next run by deleting source file
				del "%modBuildDirectory%%modName%\addons\%pboName%.pbo"
				endlocal
				exit /B 1
			)
		)
		IF EXIST "%modBuildDirectory%%singleModName%\addons\%pboName%.pbo.%keyName%.bisign" (
			del "%modBuildDirectory%%singleModName%\addons\%pboName%.pbo.%keyName%.bisign"
			IF EXIST "%modBuildDirectory%%singleModName%\addons\%pboName%.pbo.%keyName%.bisign" (
				call :LINKERROR "%pboName%.pbo.%keyName%.bisign" "%modName%" "%modBuildDirectory%%singleModName%\addons\"
				REM make sure it's dealt with next run by deleting source file
				del "%modBuildDirectory%%modName%\addons\%pboName%.pbo.%keyName%.bisign"
				endlocal
				exit /B 1
			)
		)
	)
	IF NOT EXIST "%modBuildDirectory%%singleModName%\addons\%pboName%.pbo" (
		mklink /H "%modBuildDirectory%%singleModName%\addons\%pboName%.pbo" "%modBuildDirectory%%modName%\addons\%pboName%.pbo"
	)
	IF NOT EXIST "%modBuildDirectory%%singleModName%\addons\%pboName%.pbo.%keyName%.bisign" (
		mklink /H "%modBuildDirectory%%singleModName%\addons\%pboName%.pbo.%keyName%.bisign" "%modBuildDirectory%%modName%\addons\%pboName%.pbo.%keyName%.bisign"
	)
)
if /I "%modName%" NEQ "%LicensedModName%" if defined bundleModName (
	REM Same for bundle
	if !filesChanged! NEQ 0 if /I "%modBuildDirectory%%bundleModName%" NEQ "%modBuildDirectory%%modName%" (
		IF EXIST "%modBuildDirectory%%bundleModName%\addons\%pboName%.pbo" (
			del "%modBuildDirectory%%bundleModName%\addons\%pboName%.pbo"
			IF EXIST "%modBuildDirectory%%bundleModName%\addons\%pboName%.pbo" (
				call :LINKERROR "%pboName%.pbo" "%modName%" "%modBuildDirectory%%bundleModName%\addons\"
				REM make sure it's dealt with next run by deleting source file
				del "%modBuildDirectory%%modName%\addons\%pboName%.pbo"
				endlocal
				exit /B 1
			)
		)
		IF EXIST "%modBuildDirectory%%bundleModName%\addons\%pboName%.pbo.%keyName%.bisign" (
			del "%modBuildDirectory%%bundleModName%\addons\%pboName%.pbo.%keyName%.bisign"
			IF EXIST "%modBuildDirectory%%bundleModName%\addons\%pboName%.pbo.%keyName%.bisign" (
				call :LINKERROR "%pboName%.pbo.%keyName%.bisign" "%modName%" "%modBuildDirectory%%bundleModName%\addons\"
				REM make sure it's dealt with next run by deleting source file
				del "%modBuildDirectory%%modName%\addons\%pboName%.pbo.%keyName%.bisign"
				endlocal
				exit /B 1
			)
		)
	)
	IF NOT EXIST "%workDrive%%prefixName%\BUNDLEIGNORE" (
		IF NOT EXIST "%modBuildDirectory%%bundleModName%\addons\%pboName%.pbo" (
			mklink /H "%modBuildDirectory%%bundleModName%\addons\%pboName%.pbo" "%modBuildDirectory%%modName%\addons\%pboName%.pbo"
			set bundleDeploy=1
		)
		IF NOT EXIST "%modBuildDirectory%%bundleModName%\addons\%pboName%.pbo.%keyName%.bisign" (
			mklink /H "%modBuildDirectory%%bundleModName%\addons\%pboName%.pbo.%keyName%.bisign" "%modBuildDirectory%%modName%\addons\%pboName%.pbo.%keyName%.bisign"
			set bundleDeploy=1
		)
	)
)
if !deploy! NEQ 0 (
	echo !date! !time! > "%~dp0%modPrefix%.deploy"
)
if !bundleDeploy! NEQ 0 (
	echo !date! !time! > "%~dp0Bundle.deploy"
)
:EXITPACK
endlocal
exit /B 0
REM End :PACK

:LINKERROR fileName modName destFolder
setlocal enableextensions enabledelayedexpansion
set "fileName=%~1"
set "modName=%~2"
set "destFolder=%~3"
echo /////////////////////////////////////////////////////////////
echo %date% %time% Something went wrong with %fileName% for "%modName%", it could not be hardlinked to "%destFolder%%fileName%"
echo /////////////////////////////////////////////////////////////
endlocal
exit /B 1

:SIGN

setlocal enableextensions enabledelayedexpansion

set "modName=%~1"
set "pboName=%~2"

if "%modName%"=="%clientModName%" (
	echo %date% %time% Signing "%modBuildDirectory%%modName%\addons\%pboName%.pbo"
	"%_DAYZTOOLSPATH%\DsUtils\DSSignFile.exe" "%keyDirectory%%keyName%.biprivatekey" "%modBuildDirectory%%modName%\addons\%pboName%.pbo"
) else (
	REM Have to use the clients BISign files on the server
	echo %date% %time% Copying "%modBuildDirectory%%clientModName%\addons\%pboName%.pbo.%keyName%.bisign" to "%modBuildDirectory%%serverModName%\addons\"
	copy "%modBuildDirectory%%clientModName%\addons\%pboName%.pbo.%keyName%.bisign" "%modBuildDirectory%%serverModName%\addons\"
)
exit /B
REM End :SIGN

:EXIT
del "%~dp0%modPrefix%.tmp"
endlocal
exit /B 0

:ABORT
echo %time% > "%~dp0%modPrefix%.failure"
del "%~dp0%modPrefix%.tmp"
echo %date% %time% Failed to package %modPrefix%.
endlocal
exit /B 1