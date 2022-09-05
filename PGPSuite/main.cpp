#include <iostream>
#include <raylib.h>

int main()
{
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(144);

    while (!WindowShouldClose())
    {

    }

    CloseWindow();

    return 0;
}
