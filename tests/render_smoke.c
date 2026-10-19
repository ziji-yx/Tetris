#include <stddef.h>

#define main tetris_application_main
#include "../Tetris.demo.c"
#undef main

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        return 2;
    }

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(BOARD_WIDTH + SIDEBAR_WIDTH, BOARD_HEIGHT, "Tetris render smoke test");

    highScore = 0;
    ResetGame();

    RenderTexture2D target = LoadRenderTexture(BOARD_WIDTH + SIDEBAR_WIDTH, BOARD_HEIGHT);
    BeginTextureMode(target);
    ClearBackground((Color){8, 10, 15, 255});
    DrawBoard();
    DrawSidebar();
    DrawGameOverlay();
    EndTextureMode();

    Image image = LoadImageFromTexture(target.texture);
    ImageFlipVertical(&image);
    ExportImage(image, argv[1]);

    UnloadImage(image);
    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
