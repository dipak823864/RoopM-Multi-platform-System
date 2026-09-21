@echo off
cd /d "%~dp0"

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
if %ERRORLEVEL% NEQ 0 (
    call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
)

set INCLUDES=/I"." /I"core" /I"vendor" /I"vendor/quickjs" /I"vendor/yoga" /I"vendor/lexbor"
set COMMON=/O2 /MD /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /DCONFIG_VERSION=\"2024-01-13\" /DWIN32_LEAN_AND_MEAN /DLEXBOR_STATIC

echo [1/2] Compiling True Headless Pixel Engine Test Suite...
cl %COMMON% /std:c++20 /EHsc %INCLUDES% /Febuild\test_pixel_engine_e2e.exe tests_src\test_pixel_engine_e2e.cpp vendor\yoga\yoga\*.cpp vendor\yoga\yoga\algorithm\*.cpp vendor\yoga\yoga\node\*.cpp vendor\yoga\yoga\config\*.cpp vendor\yoga\yoga\debug\*.cpp vendor\yoga\yoga\event\*.cpp build\quickjs_unity.obj build\lexbor_unity.obj user32.lib advapi32.lib shell32.lib gdi32.lib /Fobuild\
if %ERRORLEVEL% NEQ 0 ( echo [ERROR] Build Failed! & exit /b %ERRORLEVEL% )

echo.
echo [2/2] Executing True Framebuffer Pixel Test Suite...
echo.
build\test_pixel_engine_e2e.exe
