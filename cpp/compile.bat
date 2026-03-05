@echo off
echo Compiling Punic Wars: Castra...

REM Спроба компіляції з різними можливими шляхами до Raylib
if exist "C:\raylib\w64devkit\bin\g++.exe" (
    echo Using Raylib w64devkit...
    set "PATH=C:\raylib\w64devkit\bin;%PATH%"
    "C:\raylib\w64devkit\bin\g++.exe" src\main.cpp src\ui_button.cpp src\building_texture_manager.cpp src\building_renderer.cpp src\tilemap\tilemap.cpp src\tilemap\noise.cpp src\tilemap\map_generator.cpp src\tilemap\isometric_renderer.cpp src\tilemap\map_serializer.cpp -o punic_wars.exe -Isrc -I"C:\raylib\raylib\include" -L"C:\raylib\raylib\lib" -lraylib -lopengl32 -lgdi32 -lwinmm
) else if exist "C:\raylib\mingw\bin\g++.exe" (
    echo Using Raylib MinGW...
    "C:\raylib\mingw\bin\g++.exe" src\main.cpp src\ui_button.cpp src\building_texture_manager.cpp src\building_renderer.cpp src\tilemap\tilemap.cpp src\tilemap\noise.cpp src\tilemap\map_generator.cpp src\tilemap\isometric_renderer.cpp src\tilemap\map_serializer.cpp -o punic_wars.exe -Isrc -I"C:\raylib\include" -L"C:\raylib\lib" -lraylib -lopengl32 -lgdi32 -lwinmm
) else if exist "C:\MinGW\bin\g++.exe" (
    echo Using system MinGW...
    "C:\MinGW\bin\g++.exe" src\main.cpp src\ui_button.cpp src\building_texture_manager.cpp src\building_renderer.cpp src\tilemap\tilemap.cpp src\tilemap\noise.cpp src\tilemap\map_generator.cpp src\tilemap\isometric_renderer.cpp src\tilemap\map_serializer.cpp -o punic_wars.exe -Isrc -lraylib -lopengl32 -lgdi32 -lwinmm
) else (
    echo Trying system g++...
    g++ src\main.cpp src\ui_button.cpp src\building_texture_manager.cpp src\building_renderer.cpp src\tilemap\tilemap.cpp src\tilemap\noise.cpp src\tilemap\map_generator.cpp src\tilemap\isometric_renderer.cpp src\tilemap\map_serializer.cpp -o punic_wars.exe -Isrc -lraylib -lopengl32 -lgdi32 -lwinmm
)

if exist "punic_wars.exe" (
    echo Compilation successful!
    echo Run with: punic_wars.exe
) else (
    echo Compilation failed. Please check SETUP.md for installation instructions.
)