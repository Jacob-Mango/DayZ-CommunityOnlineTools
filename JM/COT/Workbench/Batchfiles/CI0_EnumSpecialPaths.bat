@echo off
setlocal enabledelayedexpansion

if not defined workDrive (
	echo ERROR: WorkDrive not defined
	exit /b 1
)

if not defined PrefixLinkRoot (
	echo ERROR: PrefixLinkRoot not defined
	exit /b 1
)

set "modPrefix=%~1"

if not defined modPrefix (
	echo ERROR: modPrefix not defined
	exit /b 1
)

echo %date% %time% Getting "special" file paths...
dir /B /S IGNORE? > "%workDrive%Temp\%PrefixLinkRoot%-%modPrefix%-specialpaths.list" 2>NUL
echo %date% %time% ...got "special" file paths
