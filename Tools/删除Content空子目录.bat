@echo off

set "rootFolder=Content"

for /f "delims=" %%d in ('dir /ad /b /s "%rootFolder%" ^| sort /r') do (
    rd "%%d" 2>nul
)

echo All empty directories have been deleted.

pause