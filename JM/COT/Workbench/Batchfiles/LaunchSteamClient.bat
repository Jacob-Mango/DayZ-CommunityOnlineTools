@echo off
setlocal

if "%~1"=="" (
	echo ERROR: No SteamID argument^!
	echo Usage: %~dpnx0 ^<SteamID^> ^<GameDirectory^> ^<GameEXE^> ^<client launch arguments^>
	exit /b 1
)

set SteamID=%~1
shift /1

if "%~1"=="" (
	echo ERROR: No GameDirectory argument^!
	echo Usage: %~dpnx0 ^<SteamID^> ^<GameDirectory^> ^<GameEXE^> ^<client launch arguments^>
	exit /b 1
)

set GameDirectory=%~1
shift /1

REM if not exist "%GameDirectory%experimental_steamclient" (
REM 	echo ERROR: "%GameDirectory%experimental_steamclient" not found^!
REM 	exit /b 1
REM )

REM if not exist "%GameDirectory%steam_appid.txt" (
REM 	echo ERROR: "%GameDirectory%steam_appid.txt" not found^!
REM 	exit /b 1
REM )

if "%~1"=="" (
	echo ERROR: No GameEXE argument^!
	echo Usage: %~dpnx0 ^<SteamID^> ^<GameDirectory^> ^<GameEXE^> ^<client launch arguments^>
	exit /b 1
)

set GameEXE=%~1
shift /1

if "%~1"=="" (
	echo ERROR: No client launch arguments^!
	echo Usage: %~nx0 ^<SteamID^> ^<GameDirectory^> ^<GameEXE^> ^<client launch arguments^>
	exit /b 1
)

:getargs
if "%~1"=="" goto main
if defined launchArgs (
	set launchArgs=%launchArgs% %1
) else (
	set launchArgs=%1
)
set arg=%~1
if "%arg:~0,5%"=="-name" (
	set "AccountName=%arg:~6%"
)
shift /1
goto getargs

:main
REM echo [SteamClient]>"%GameDirectory%experimental_steamclient\ColdClientLoader.ini"
REM echo Exe=%GameDirectory%%GameEXE%>>"%GameDirectory%experimental_steamclient\ColdClientLoader.ini"
REM echo ExeRunDir=%GameDirectory%>>"%GameDirectory%experimental_steamclient\ColdClientLoader.ini"
REM echo ExeCommandLine=%launchArgs%>>"%GameDirectory%experimental_steamclient\ColdClientLoader.ini"
REM set /p AppId=<"%GameDirectory%steam_appid.txt"
REM set AppId=%AppId: =%
REM echo AppId=%AppId%>>"%GameDirectory%experimental_steamclient\ColdClientLoader.ini"
REM echo.>>"%GameDirectory%experimental_steamclient\ColdClientLoader.ini"
REM echo SteamClientDll=steamclient.dll>>"%GameDirectory%experimental_steamclient\ColdClientLoader.ini"
REM echo SteamClient64Dll=steamclient64.dll>>"%GameDirectory%experimental_steamclient\ColdClientLoader.ini"

if not defined AccountName set AccountName=Noob
echo AccountName: %AccountName%
echo SteamID: %SteamID%
if not exist "%APPDATA%\Goldberg SteamEmu Saves\settings" mkdir "%APPDATA%\Goldberg SteamEmu Saves\settings"
echo | set /p ="%AccountName%">"%APPDATA%\Goldberg SteamEmu Saves\settings\account_name.txt"
echo | set /p ="%SteamID%">"%APPDATA%\Goldberg SteamEmu Saves\settings\user_steam_id.txt"

echo "%GameDirectory%%GameEXE%" %launchArgs%
REM start "" /D "%gameDirectory%experimental_steamclient" "%GameDirectory%experimental_steamclient\steamclient_loader.exe"
start "" /D "%GameDirectory%" %GameEXE% %launchArgs%
