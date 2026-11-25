@echo off
setlocal enabledelayedexpansion

:: Gather all .c files in the src directory
set SOURCES=
for %%f in (src\*.c) do (
    set SOURCES=!SOURCES! "%%f"
)

:: Compile with -Isrc so GCC finds any .h files inside src
gcc -std=c11 -Wall -Wextra -Wpedantic -Isrc -o perspecta.exe %SOURCES%

:: Check if the build succeeded or failed
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)

echo Build succeeded: perspecta.exe
