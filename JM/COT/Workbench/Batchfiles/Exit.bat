@echo off

taskkill /F /IM DayZ_x64.exe >nul 2>&1
taskkill /F /IM DayZServer_x64.exe >nul 2>&1
taskkill /F /IM DZSALModServer.exe >nul 2>&1
taskkill /F /IM DayZDiag_x64.exe >nul 2>&1

taskkill /F /IM DayZDiag_x64_ClientA.exe >nul 2>&1
taskkill /F /IM DayZDiag_x64_ClientB.exe >nul 2>&1
