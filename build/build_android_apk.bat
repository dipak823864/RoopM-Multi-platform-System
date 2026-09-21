@echo off
setlocal enabledelayedexpansion
REM RoopM Web Engine - Production Android NativeActivity Split APK Packager

echo =======================================================
echo    RoopM Engine - Production Android APK Build
echo =======================================================

cd /d "%~dp0.."

set "JAVA_HOME=C:\Program Files\Android\Android Studio\jbr"
set "PATH=%JAVA_HOME%\bin;%PATH%"

set "ADB=%LOCALAPPDATA%\Android\Sdk\platform-tools\adb.exe"
set "BUILD_TOOLS=%LOCALAPPDATA%\Android\Sdk\build-tools\34.0.0"
if not exist "%BUILD_TOOLS%\aapt2.exe" (
    for /d %%i in ("%LOCALAPPDATA%\Android\Sdk\build-tools\*") do (
        if exist "%%i\aapt2.exe" set "BUILD_TOOLS=%%i"
    )
)

set "AAPT2=%BUILD_TOOLS%\aapt2.exe"
set "ZIPALIGN=%BUILD_TOOLS%\zipalign.exe"
set "APKSIGNER=%BUILD_TOOLS%\apksigner.bat"

set "ANDROID_JAR=%LOCALAPPDATA%\Android\Sdk\platforms\android-34\android.jar"
if not exist "%ANDROID_JAR%" (
    for /d %%i in ("%LOCALAPPDATA%\Android\Sdk\platforms\android-*") do (
        if exist "%%i\android.jar" set "ANDROID_JAR=%%i\android.jar"
    )
)

echo [1/4] Checking Connected Android Device via ADB...
set "DEVICE_ABI="
set "DEVICE_CONNECTED=0"
if exist "%ADB%" (
    "%ADB%" devices | findstr /r "device$" >nul
    if !ERRORLEVEL! EQU 0 (
        set "DEVICE_CONNECTED=1"
        for /f "delims=" %%a in ('"%ADB%" shell getprop ro.product.cpu.abi 2^>nul') do (
            set "DEVICE_ABI=%%a"
        )
        echo   -- USB Device Connected! Architecture: !DEVICE_ABI!
    )
)

if "!DEVICE_ABI!"=="" set "DEVICE_ABI=arm64-v8a"

set "SO_FILE=android_build\libs\!DEVICE_ABI!\libroopm_app.so"
if not exist "!SO_FILE!" (
    echo [2/4] Compiling Full Web Engine for !DEVICE_ABI!...
    call build\build_android.bat "!DEVICE_ABI!"
) else (
    echo [2/4] Using pre-compiled shared library: !SO_FILE!
)

if not exist "!SO_FILE!" (
    echo 🚨 [ERROR] libroopm_app.so not found!
    exit /b 1
)

echo.
echo [3/4] Packaging Official APK with Assets (workspace/)...
set "WORK_DIR=android_build\apk_work_!DEVICE_ABI!"
if exist "!WORK_DIR!" rmdir /s /q "!WORK_DIR!"
mkdir "!WORK_DIR!" 2>nul
mkdir "!WORK_DIR!\lib\!DEVICE_ABI!" 2>nul
mkdir "!WORK_DIR!\apk_assets\workspace" 2>nul

copy "!SO_FILE!" "!WORK_DIR!\lib\!DEVICE_ABI!\" > nul
xcopy /E /I /Y "workspace" "!WORK_DIR!\apk_assets\workspace\" > nul

(
echo ^<?xml version="1.0" encoding="utf-8"?^>
echo ^<manifest xmlns:android="http://schemas.android.com/apk/res/android"
echo     package="com.roopm.demo"
echo     android:versionCode="1"
echo     android:versionName="1.0"^>
echo.
echo     ^<uses-sdk android:minSdkVersion="21" android:targetSdkVersion="34" /^>
echo.
echo     ^<uses-permission android:name="android.permission.INTERNET" /^>
echo     ^<uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" /^>
echo.
echo     ^<application
echo         android:label="RoopM"
echo         android:hasCode="false"
echo         android:allowBackup="false"
echo         android:hardwareAccelerated="true"
echo         android:theme="@android:style/Theme.NoTitleBar.Fullscreen"^>
echo.
echo         ^<activity
echo             android:name="android.app.NativeActivity"
echo             android:label="RoopM"
echo             android:exported="true"
echo             android:launchMode="singleTask"
echo             android:screenOrientation="unspecified"
echo             android:windowSoftInputMode="adjustResize"
echo             android:configChanges="mcc|mnc|locale|touchscreen|keyboard|keyboardHidden|navigation|orientation|screenLayout|uiMode|screenSize|smallestScreenSize|layoutDirection|fontScale|density|colorMode|fontWeightAdjustment|grammaticalGender"^>
echo.
echo             ^<meta-data android:name="android.app.lib_name" android:value="roopm_app" /^>
echo.
echo             ^<intent-filter^>
echo                 ^<action android:name="android.intent.action.MAIN" /^>
echo                 ^<category android:name="android.intent.category.LAUNCHER" /^>
echo             ^</intent-filter^>
echo         ^</activity^>
echo     ^</application^>
echo ^</manifest^>
) > "!WORK_DIR!\AndroidManifest.xml"

"%AAPT2%" link ^
    --manifest "!WORK_DIR!\AndroidManifest.xml" ^
    -A "!WORK_DIR!\apk_assets" ^
    -I "%ANDROID_JAR%" ^
    -o "android_build\base_!DEVICE_ABI!.apk" ^
    --min-sdk-version 21 ^
    --target-sdk-version 34

pushd "!WORK_DIR!"
jar -uvf "..\base_!DEVICE_ABI!.apk" "lib/!DEVICE_ABI!/libroopm_app.so" > nul
popd

"%ZIPALIGN%" -f -p 4 "android_build\base_!DEVICE_ABI!.apk" "android_build\aligned_!DEVICE_ABI!.apk"

if not exist "android_build\debug.keystore" (
    keytool -genkeypair -v -keystore "android_build\debug.keystore" ^
        -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 ^
        -storepass android -keypass android ^
        -dname "CN=Debug,O=Android,C=US" 2>nul
)

call "%APKSIGNER%" sign ^
    --ks "android_build\debug.keystore" ^
    --ks-pass pass:android ^
    --ks-key-alias androiddebugkey ^
    --key-pass pass:android ^
    --out "android_build\roopm_demo_!DEVICE_ABI!.apk" ^
    "android_build\aligned_!DEVICE_ABI!.apk" 2>nul

if exist "android_build\roopm_demo_!DEVICE_ABI!.apk" (
    for %%F in ("android_build\roopm_demo_!DEVICE_ABI!.apk") do (
        set /a "SIZE_BYTES=%%~zF"
        set /a "SIZE_KB=SIZE_BYTES / 1024"
        echo.
        echo =======================================================
        echo  🎉 [SUCCESS] APK Generated: android_build\roopm_demo_!DEVICE_ABI!.apk
        echo  📦 Final Production APK Size: !SIZE_KB! KB [!SIZE_BYTES! bytes]
        echo =======================================================
    )
)

echo.
echo [4/4] USB Auto-Deployment to Device...
if "!DEVICE_CONNECTED!"=="1" (
    echo   -- Installing APK to device...
    "%ADB%" install -r "android_build\roopm_demo_!DEVICE_ABI!.apk"
    echo   -- Launching RoopM NativeActivity...
    "%ADB%" shell am start -n com.roopm.demo/android.app.NativeActivity
    echo   ✨ [DONE] App is now running LIVE on your Android phone!
)
