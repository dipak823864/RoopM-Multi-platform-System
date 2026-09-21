@echo off
cd /d "%~dp0"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
if %ERRORLEVEL% NEQ 0 (
    call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
)

set INCLUDES=/I"." /I"core" /I"vendor" /I"vendor/quickjs" /I"vendor/yoga" /I"vendor/lexbor" /I"vendor/blend2d"
set COMMON=/O2 /MD /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /DCONFIG_VERSION=\"2024-01-13\" /DWIN32_LEAN_AND_MEAN /DLEXBOR_STATIC /DBL_STATIC /DBL_BUILD_NO_JIT

echo Compiling and Linking Complete RoopM Web Engine with Native Blend2D...
cl %COMMON% /std:c++20 /EHsc %INCLUDES% /Febuild\roopm_demo.exe app\app.cpp core\roopm_render.cpp core\roopm_font.cpp core\roopm_image.cpp platform\windows\platform_win32.cpp build\blend2d_unity.obj build\sizetable.obj build\checksum_sse4_2.obj build\otglyf_sse4_2.obj build\pixelconverter_ssse3.obj vendor\yoga\yoga\*.cpp vendor\yoga\yoga\algorithm\*.cpp vendor\yoga\yoga\node\*.cpp vendor\yoga\yoga\config\*.cpp vendor\yoga\yoga\debug\*.cpp vendor\yoga\yoga\event\*.cpp build\quickjs_unity.obj build\lexbor_unity.obj user32.lib advapi32.lib shell32.lib gdi32.lib shcore.lib /Fobuild\

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Launching Live Web Engine with Native Blend2D Canvas!
    start "" build\roopm_demo.exe
)
