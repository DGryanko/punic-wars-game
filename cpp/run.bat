@echo off
if exist "punic_wars.exe" (
    echo Running Punic Wars: Castra...
    punic_wars.exe
) else (
    echo Game not compiled yet. Run compile.bat first.
)