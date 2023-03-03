@echo off

set /p enableCompression=Enable Compression?[Y/N]?

if /I "%enableCompression%"=="Y" ( 
	set compression=compress
) else (
	set compression=
)

set /p enableShrink=Enable Shrink?[Y/N]?

if /I "%enableShrink%"=="Y" ( 
	set shrink=shrinkP3D
) else (
	set shrink=
)

REM TODO actually implement the compression and shrink options in CI1.bat, or maybe not ^^
call "%~dp0CI.bat" %compression% %shrink% %*
