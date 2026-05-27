@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo   Linear Programming Solver - Install
echo ========================================
echo.

cd /d "%~dp0"

:: ========================================
:: CHECK AND INSTALL PYTHON
:: ========================================
echo [1/3] Checking Python...

set PYTHON_CMD=

:: Python Launcher (py)
py --version >nul 2>&1
if !errorlevel! equ 0 (
    set PYTHON_CMD=py
    for /f "tokens=*" %%v in ('py --version 2^>^&1') do echo [OK] Found: %%v
    goto :pip_install
)

:: Common paths
for %%p in (
    "%LOCALAPPDATA%\Programs\Python\Python313\python.exe"
    "%LOCALAPPDATA%\Programs\Python\Python312\python.exe"
    "C:\Python313\python.exe"
    "C:\Python312\python.exe"
) do (
    if exist "%%p" (
        "%%p" --version >nul 2>&1
        if !errorlevel! equ 0 (
            set PYTHON_CMD="%%p"
            for /f "tokens=*" %%v in ('"%%p" --version 2^>^&1') do echo [OK] Found: %%v
            goto :pip_install
        )
    )
)

echo.
echo [ERROR] Python not found!
echo Please install Python from: https://www.python.org/downloads/
pause
exit /b 1

:pip_install

:: ========================================
:: INSTALL PACKAGES
:: ========================================
echo.
echo [2/3] Installing packages...

!PYTHON_CMD! -m pip install --upgrade pip --quiet --disable-pip-version-check 2>nul
!PYTHON_CMD! -m pip install numpy matplotlib --quiet --disable-pip-version-check 2>nul
if !errorlevel! neq 0 (
    !PYTHON_CMD! -m pip install numpy matplotlib --user --quiet --disable-pip-version-check 2>nul
)
echo [OK] Packages installed

:: ========================================
:: COMPILE C++ SOLVER
:: ========================================
echo.
echo [3/3] Compiling C++ Solver...

if not exist "src\LPInterface.cpp" (
    echo [ERROR] Source files not found!
    pause
    exit /b 1
)

g++ --version >nul 2>&1
if !errorlevel! neq 0 (
    echo.
    echo [ERROR] GCC compiler not found!
    echo.
    echo Please install MinGW-w64 from: https://www.mingw-w64.org/
    echo Or use MSYS2 from: https://www.msys2.org/
    pause
    exit /b 1
)

cd src
g++ -std=c++17 -o LPInterface LPInterface.cpp -Iinclude -Wall 2>nul
if !errorlevel! neq 0 (
    g++ -std=c++17 -o LPInterface LPInterface.cpp -I. -Wall 2>nul
    if !errorlevel! neq 0 (
        cd ..
        echo [ERROR] Compilation failed!
        pause
        exit /b 1
    )
)
cd ..

echo [OK] C++ Solver compiled

echo.
echo ========================================
echo   INSTALLATION COMPLETE!
echo ========================================
echo.
echo Run LPSolver.bat to start the application.
echo.
pause
