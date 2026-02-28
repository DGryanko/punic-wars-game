#include "raylib.h"
#include <stdio.h>

int main() {
    InitWindow(800, 600, "Texture Test");
    
    Texture2D cursor = LoadTexture("assets/sprites/Cursor_idle.png");
    Texture2D panel = LoadTexture("assets/sprites/ResourcePanel.png");
    
    printf("Cursor ID: %d\n", cursor.id);
    printf("Panel ID: %d\n", panel.id);
    
    if (cursor.id > 0) {
        printf("Cursor loaded: %dx%d\n", cursor.width, cursor.height);
    } else {
        printf("Failed to load cursor\n");
    }
    
    if (panel.id > 0) {
        printf("Panel loaded: %dx%d\n", panel.width, panel.height);
    } else {
        printf("Failed to load panel\n");
    }
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        
        if (cursor.id > 0) {
            DrawTexture(cursor, 100, 100, WHITE);
        }
        
        if (panel.id > 0) {
            DrawTexture(panel, 100, 200, WHITE);
        }
        
        DrawText("Press ESC to exit", 10, 10, 20, WHITE);
        
        EndDrawing();
    }
    
    UnloadTexture(cursor);
    UnloadTexture(panel);
    CloseWindow();
    
    return 0;
}
