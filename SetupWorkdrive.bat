@echo off
IF NOT exist "P:\JM\" (
	echo Creating folder P:\JM\
	mkdir "P:\JM"
)

IF exist "P:\JM\COT\" (
	echo Removing existing link P:\JM\COT
	rmdir "P:\JM\COT\"
)

echo Creating link P:\JM\COT
mklink /J "P:\JM\COT\" "%cd%\JM\COT\"

echo Done