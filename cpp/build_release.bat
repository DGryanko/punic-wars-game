@echo off
chcp 65001 >nul
echo ============================================
echo  Punic Wars: Castra - Build Release
echo ============================================
echo.

:: Крок 1: Компіляція
echo [1/2] Compiling game...
call "%~dp0compile.bat"
if errorlevel 1 (
    echo ERROR: Compilation failed!
    pause
    exit /b 1
)
echo.

:: Крок 2: Пакування (інсталятор + portable ZIP)
echo [2/2] Building installer and portable ZIP...
powershell -ExecutionPolicy Bypass -File "%~dp0installer\build_installer.ps1"
if errorlevel 1 (
    echo ERROR: Installer build failed!
    pause
    exit /b 1
)

echo.
echo ============================================
echo  Done! Check cpp\installer\output\
echo ============================================
pause
