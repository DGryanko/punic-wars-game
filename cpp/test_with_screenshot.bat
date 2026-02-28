@echo off
echo === Punic Wars: Castra - Auto Screenshot ===
echo.

REM Перевірка чи існує exe
if not exist "punic_wars.exe" (
    echo Error: punic_wars.exe not found!
    echo Run compile.bat first
    pause
    exit /b 1
)

REM Створюємо папку для скріншотів
if not exist "screenshots" mkdir screenshots

echo Starting game...
start "" "punic_wars.exe"

echo Waiting 5 seconds for game to load...
timeout /t 5 /nobreak >nul

echo.
echo Game is running!
echo Please take a screenshot manually (Alt+PrtScn or use Snipping Tool)
echo Screenshots folder: %CD%\screenshots
echo.
echo Press any key to continue...
pause >nul

echo.
echo To close the game, close the game window or press Ctrl+C here
