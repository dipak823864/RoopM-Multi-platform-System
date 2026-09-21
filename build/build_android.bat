@echo off
REM RoopM Web Engine - Complete Blend2D ARM64 NDK Build

cd /d "%~dp0.."

set "TARGET_ABI=%~1"
if "%TARGET_ABI%"=="" set "TARGET_ABI=arm64-v8a"

set "NDK=%LOCALAPPDATA%\Android\Sdk\ndk\27.0.12077973"
if not exist "%NDK%" (
    set "NDK=%LOCALAPPDATA%\Android\Sdk\ndk\29.0.13599879"
)
if not exist "%NDK%" (
    for /d %%i in ("%LOCALAPPDATA%\Android\Sdk\ndk\*") do set "NDK=%%i"
)

if not exist "%NDK%\ndk-build.cmd" (
    echo ERROR: Android NDK not found!
    exit /b 1
)

echo Using NDK: %NDK%
echo Target ABI: %TARGET_ABI%

if not exist android_build mkdir android_build
if not exist android_build\jni mkdir android_build\jni

if not exist android_build\jni\Android.mk (
echo LOCAL_PATH := $(call my-dir^)
echo.
echo # ==========================================================
echo # Module 1: blend2d_asimd (Dedicated ARM64 ASIMD Kernels^)
echo # ==========================================================
echo include $(CLEAR_VARS^)
echo LOCAL_MODULE := blend2d_asimd
echo LOCAL_C_INCLUDES := $(LOCAL_PATH^)/../.. \
echo                    $(LOCAL_PATH^)/../../vendor \
echo                    $(LOCAL_PATH^)/../../vendor/blend2d
echo LOCAL_CFLAGS := -std=c11 -O2 -flto -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1 -DBL_BUILD_OPT_ASIMD=1 -DBL_TARGET_OPT_ASIMD -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden
echo LOCAL_CPPFLAGS := -std=c++20 -O2 -flto -fno-exceptions -fno-rtti -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1 -DBL_BUILD_OPT_ASIMD=1 -DBL_TARGET_OPT_ASIMD -DNDEBUG
echo LOCAL_SRC_FILES := ../../vendor/blend2d/codec/pngops_asimd.cpp \
echo                    ../../vendor/blend2d/compression/checksum_asimd.cpp \
echo                    ../../vendor/blend2d/opentype/otglyf_asimd.cpp
echo include $(BUILD_STATIC_LIBRARY^)
echo.
echo # ==========================================================
echo # Module 2: roopm_app (Complete Web Engine with Blend2D^)
echo # ==========================================================
echo include $(CLEAR_VARS^)
echo LOCAL_MODULE := roopm_app
echo.
echo LOCAL_C_INCLUDES := $(LOCAL_PATH^)/../.. \
echo                    $(LOCAL_PATH^)/../../core \
echo                    $(LOCAL_PATH^)/../../vendor \
echo                    $(LOCAL_PATH^)/../../vendor/quickjs \
echo                    $(LOCAL_PATH^)/../../vendor/yoga \
echo                    $(LOCAL_PATH^)/../../vendor/lexbor \
echo                    $(LOCAL_PATH^)/../../vendor/blend2d
echo.
echo LOCAL_CFLAGS := -std=c11 -O2 -flto -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1 -DBL_BUILD_OPT_ASIMD=1 -DCONFIG_VERSION=\"2024-01-13\" -DLEXBOR_STATIC -D_GNU_SOURCE
echo LOCAL_CPPFLAGS := -std=c++20 -O2 -flto -fno-exceptions -fno-rtti -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1 -DBL_BUILD_OPT_ASIMD=1 -DCONFIG_VERSION=\"2024-01-13\" -DLEXBOR_STATIC
echo LOCAL_LDFLAGS := -Wl,--gc-sections -Wl,--strip-all -flto -u ANativeActivity_onCreate
echo.
echo LOCAL_SRC_FILES := ../../core/roopm_render.cpp \
echo                    ../../core/roopm_font.cpp \
echo                    ../../core/roopm_image.cpp \
echo                    ../../platform/android/platform_android.cpp \
echo                    ../../app/app.cpp \
echo                    ../../tests_src/quickjs_unity.c \
echo                    ../../tests_src/lexbor_unity.c \
echo                    ../../tests_src/blend2d_unity.cpp \
echo                    ../../vendor/blend2d/geometry/sizetable.cpp \
echo                    ../../vendor/yoga/yoga/YGConfig.cpp \
echo                    ../../vendor/yoga/yoga/YGEnums.cpp \
echo                    ../../vendor/yoga/yoga/YGNode.cpp \
echo                    ../../vendor/yoga/yoga/YGNodeLayout.cpp \
echo                    ../../vendor/yoga/yoga/YGNodeStyle.cpp \
echo                    ../../vendor/yoga/yoga/YGPixelGrid.cpp \
echo                    ../../vendor/yoga/yoga/YGValue.cpp \
echo                    ../../vendor/yoga/yoga/algorithm/AbsoluteLayout.cpp \
echo                    ../../vendor/yoga/yoga/algorithm/Baseline.cpp \
echo                    ../../vendor/yoga/yoga/algorithm/Cache.cpp \
echo                    ../../vendor/yoga/yoga/algorithm/CalculateLayout.cpp \
echo                    ../../vendor/yoga/yoga/algorithm/FlexLine.cpp \
echo                    ../../vendor/yoga/yoga/algorithm/PixelGrid.cpp \
echo                    ../../vendor/yoga/yoga/config/Config.cpp \
echo                    ../../vendor/yoga/yoga/debug/AssertFatal.cpp \
echo                    ../../vendor/yoga/yoga/debug/Log.cpp \
echo                    ../../vendor/yoga/yoga/event/event.cpp \
echo                    ../../vendor/yoga/yoga/node/LayoutResults.cpp \
echo                    ../../vendor/yoga/yoga/node/Node.cpp
echo.
echo LOCAL_STATIC_LIBRARIES := blend2d_asimd android_native_app_glue
echo LOCAL_LDLIBS := -llog -landroid -lm
echo include $(BUILD_SHARED_LIBRARY^)
echo.
echo $(call import-module,android/native_app_glue^)
) > android_build\jni\Android.mk

if not exist android_build\jni\Application.mk if not exist android_build\jni\Application.mk (
echo APP_ABI := %TARGET_ABI%
echo APP_PLATFORM := android-21
echo APP_STL := c++_static
echo APP_OPTIM := release
echo APP_CFLAGS := -std=c11 -O2 -flto -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1
echo APP_CPPFLAGS := -std=c++20 -O2 -flto -fno-exceptions -fno-rtti -DBL_STATIC=1 -DBL_BUILD_NO_JIT=1
echo APP_LDFLAGS := -Wl,--gc-sections -Wl,--strip-all -flto
) > android_build\jni\Application.mk

echo Compiling Full Blend2D ARM64 Engine with NDK Clang...
pushd android_build
call "%NDK%\ndk-build.cmd" NDK_PROJECT_PATH=. NDK_APPLICATION_MK=jni/Application.mk -j8
popd

set "LLVM_STRIP=%NDK%\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-strip.exe"
if exist "%LLVM_STRIP%" (
    for /r "android_build\libs" %%f in (libroopm_app.so) do (
        if exist "%%f" "%LLVM_STRIP%" --strip-all "%%f" 2>nul
    )
)
