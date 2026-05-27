@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo   Linear Programming Solver
echo ========================================
echo.

cd /d "%~dp0"

:: Find Python
set PYTHON_CMD=

py --version >nul 2>&1
if !errorlevel! equ 0 set PYTHON_CMD=py

if "!PYTHON_CMD!"=="" (
    for %%p in (
        "%LOCALAPPDATA%\Programs\Python\Python313\python.exe"
        "%LOCALAPPDATA%\Programs\Python\Python312\python.exe"
        "C:\Python313\python.exe"
        "C:\Python312\python.exe"
    ) do (
        if exist "%%p" (
            "%%p" --version >nul 2>&1
            if !errorlevel! equ 0 set PYTHON_CMD="%%p"
        )
    )
)

if "!PYTHON_CMD!"=="" (
    echo [ERROR] Python not found!
    echo Please run install.bat first!
    pause
    exit /b 1
)

:: Check if solver exists
if not exist "src\LPInterface.exe" if not exist "src\LPInterface" (
    echo [WARNING] Solver not compiled!
    echo Running install.bat...
    call install.bat
)

:: Run GUI
echo Starting GUI...
echo.

!PYTHON_CMD! gui\lp_gui.py
set EXIT_CODE=!errorlevel!

if !EXIT_CODE! neq 0 (
    echo.
    echo [ERROR] GUI exited with code: !EXIT_CODE!
    pause
    exit /b !EXIT_CODE!
)

echo.
echo [OK] Closed successfully!
pause
