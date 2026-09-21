@echo off
cd /d "%~dp0"

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
if %ERRORLEVEL% NEQ 0 (
    call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
)

set INCLUDES=/I"." /I"core" /I"vendor" /I"vendor/quickjs" /I"vendor/yoga" /I"vendor/lexbor" /I"vendor/blend2d"
set COMMON=/O2 /MD /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /DCONFIG_VERSION=\"2024-01-13\" /DWIN32_LEAN_AND_MEAN /DLEXBOR_STATIC /DBL_STATIC /DBL_BUILD_NO_JIT

echo [1/2] Compiling Master Test Runner...
cl %COMMON% /std:c++20 /EHsc %INCLUDES% /Febuild\test_runner.exe tests_src\test_master_suite.cpp build\quickjs_unity.obj build\lexbor_unity.obj build\blend2d_unity.obj build\sizetable.obj build\checksum_sse4_2.obj build\otglyf_sse4_2.obj build\pixelconverter_ssse3.obj build\YGNode.obj build\YGConfig.obj build\YGEnums.obj build\YGNodeLayout.obj build\YGNodeStyle.obj build\YGPixelGrid.obj build\YGValue.obj build\AbsoluteLayout.obj build\Baseline.obj build\Cache.obj build\CalculateLayout.obj build\Config.obj build\event.obj build\FlexLine.obj build\LayoutResults.obj build\Log.obj build\Node.obj build\PixelGrid.obj build\AssertFatal.obj user32.lib advapi32.lib shell32.lib gdi32.lib shcore.lib /Fobuild\
if %ERRORLEVEL% NEQ 0 ( echo [ERROR] Test Compilation Failed! & exit /b %ERRORLEVEL% )

echo.
echo [2/2] Running Complete Master Test Suite...
echo.
build\test_runner.exe
