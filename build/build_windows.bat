@echo off
pushd "%~dp0.."

:: Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

echo.
echo ==========================================
echo    Roopm Engine - Windows Build
echo ==========================================
echo.

set SRC=core\roopm_render.cpp core\roopm_font.cpp platform\windows\platform_win32.cpp app\app.cpp
set OUT=build\roopm_demo.exe
set CFLAGS=/EHsc /O2 /std:c++17 /MT /DNOMINMAX
set INCLUDES=/I"."
set LIBS=user32.lib gdi32.lib

echo Compiling Roopm Demo...
cl %CFLAGS% %INCLUDES% %SRC% /Fe:%OUT% /link %LIBS% /SUBSYSTEM:WINDOWS > build\build_log.txt 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed! Check build\build_log.txt
    type build\build_log.txt
    popd
    pause
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Build complete: %OUT%
echo.
echo Run with: %OUT%
popd
