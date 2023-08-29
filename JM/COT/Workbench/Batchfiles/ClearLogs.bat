@echo off

del /s /q /f %serverProfileDirectory%\*.rpt
del /s /q /f %serverProfileDirectory%\*.log
del /s /q /f %serverProfileDirectory%\*.mdmp
del /s /q /f %serverProfileDirectory%\*.ADM

del /s /q /f %localappdata%\DayZ\*.rpt
del /s /q /f %localappdata%\DayZ\*.log
del /s /q /f %localappdata%\DayZ\*.mdmp
del /s /q /f %localappdata%\DayZ\*.ADM

del /s /q /f "%localappdata%\DayZ Exp\*.rpt"
del /s /q /f "%localappdata%\DayZ Exp\*.log"
del /s /q /f "%localappdata%\DayZ Exp\*.mdmp"
del /s /q /f "%localappdata%\DayZ Exp\*.ADM"