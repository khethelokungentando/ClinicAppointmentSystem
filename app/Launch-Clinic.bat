
@echo off
title Community Clinic Appointment System
cd /d "%~dp0"

echo ==========================================
echo   COMMUNITY CLINIC APPOINTMENT SYSTEM
echo ==========================================
echo.

if not exist "clinic.exe" (
    echo ERROR: clinic.exe was not found.
    echo Compile your C++ project first.
    pause
    exit /b 1
)

echo Starting the Community Clinic server...
echo.
echo Please keep this window open.
echo Press Ctrl+C to stop the application.
echo.

start "" /min cmd /c "timeout /t 3 /nobreak >nul & start http://127.0.0.1:8765"

clinic.exe

echo.
echo The application has stopped.
pause
