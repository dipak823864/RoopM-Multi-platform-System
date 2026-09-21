@echo off
REM પ્રોજેક્ટના મુખ્ય (Root) ફોલ્ડરમાં જાઓ
cd /d "%~dp0.."

echo ==========================================
echo    RoopM Engine - Web Build
echo ==========================================

echo [Tool] Compiling C# Web Builder...
"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe" /nologo /out:tools\RoopmWebBuilder.exe tools\RoopmWebBuilder.cs
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] C# Compiler failed!
    pause
    exit /b %ERRORLEVEL%
)

echo [Build] Generating Smart Web App...
tools\RoopmWebBuilder.exe

echo.
pause