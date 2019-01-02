@ech off
set /p REVISION=<REVISION.txt
set /a REVISION=%Build%+1
echo %REVISION%>REVISION.txt
pause