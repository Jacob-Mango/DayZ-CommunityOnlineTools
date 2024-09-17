set modList=

if not defined mods exit /b

for %%a in ("%mods:;=" "%") do (
    if /i "%%~dpnxa"=="%%~a" (
        REM mod does contain path
        if not defined modList (
            set modList=%%~a
        ) else (
            call :addmodtolist "%%~a"
        )
    ) else (
        REM mod does not contain path
        if not defined modList (
            set modList=%modBuildDirectory%%%~a
        ) else (
            call :addmodtolist "%modBuildDirectory%%%~a"
        )
    )
)

exit /b

:addmodtolist
set modList=%modList%;%~1
exit /b
