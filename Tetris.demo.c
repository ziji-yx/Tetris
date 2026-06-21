#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// --- 游戏配置 ---
#define COLS 10
#define ROWS 20
#define CELL_SIZE 40

// --- 结构体定义 ---
typedef struct
{
    int id;
    Color color;
    int matrix[4][4];
} Tetromino;

typedef struct
{
    int x, y;
    Tetromino type;
} Player;

// --- 全局变量 ---
int grid[ROWS][COLS] = {0};
Color gridColors[ROWS][COLS];
int score = 0;
int linesCleared = 0;

// 定义 7 种标准方块
Tetromino shapes[7] = {
    {1, {0, 121, 241, 255}, {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}},   // I
    {2, {0, 82, 172, 255}, {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},    // J
    {3, {255, 161, 0, 255}, {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},   // L
    {4, {253, 249, 0, 255}, {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},   // O
    {5, {0, 228, 48, 255}, {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},    // S
    {6, {200, 122, 255, 255}, {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}}, // T
    {7, {230, 41, 55, 255}, {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}}    // Z
};

Tetromino nextPiece; // 存储下一个方块

// --- 功能函数 ---

void DrawFancyBlock(int x, int y, Color baseColor, int size)
{
    int px = x * size;
    int py = y * size;
    DrawRectangleGradientV(px + 1, py + 1, size - 2, size - 2, ColorBrightness(baseColor, 0.4f), baseColor);
    DrawRectangleLines(px + 1, py + 1, size - 2, size - 2, Fade(WHITE, 0.3f));
    DrawRectangle(px + 4, py + 4, 5, 5, Fade(WHITE, 0.4f));
}

bool CheckCollision(int nx, int ny, int mat[4][4])
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (mat[r][c])
            {
                int gx = nx + c;
                int gy = ny + r;
                if (gx < 0 || gx >= COLS || gy >= ROWS)
                    return true;
                if (gy >= 0 && grid[gy][gx])
                    return true;
            }
        }
    }
    return false;
}

void RotatePiece(Player *p)
{
    int nextMat[4][4] = {0};
    int n = (p->type.id == 1 || p->type.id == 4) ? 4 : 3;
    for (int r = 0; r < n; r++)
        for (int c = 0; c < n; c++)
            nextMat[c][n - 1 - r] = p->type.matrix[r][c];
    if (!CheckCollision(p->x, p->y, nextMat))
        memcpy(p->type.matrix, nextMat, sizeof(int) * 16);
}

void ClearLines()
{
    int consecutiveLines = 0;
    for (int r = ROWS - 1; r >= 0; r--)
    {
        bool full = true;
        for (int c = 0; c < COLS; c++)
            if (grid[r][c] == 0)
                full = false;
        if (full)
        {
            consecutiveLines++;
            for (int tr = r; tr > 0; tr--)
            {
                for (int tc = 0; tc < COLS; tc++)
                {
                    grid[tr][tc] = grid[tr - 1][tc];
                    gridColors[tr][tc] = gridColors[tr - 1][tc];
                }
            }
            memset(grid[0], 0, sizeof(int) * COLS);
            r++;
        }
    }
    // 分数计算逻辑：消行越多倍率越高
    if (consecutiveLines > 0)
    {
        linesCleared += consecutiveLines;
        score += (consecutiveLines * consecutiveLines) * 100;
    }
}

void ResetPiece(Player *p)
{
    static bool firstCall = true;
    if (firstCall)
    {
        nextPiece = shapes[GetRandomValue(0, 6)];
        firstCall = false;
    }
    p->type = nextPiece;
    nextPiece = shapes[GetRandomValue(0, 6)]; // 预生成下一个
    p->x = COLS / 2 - 2;
    p->y = 0;
    if (CheckCollision(p->x, p->y, p->type.matrix))
    {
        memset(grid, 0, sizeof(grid));
        score = 0;
        linesCleared = 0;
    }
}

int main()
{
    const int sidebarWidth = 400; 
    InitWindow(COLS * CELL_SIZE + sidebarWidth, ROWS * CELL_SIZE, "Tetris Pro - unifp");
    SetTargetFPS(60);

    Player player;
    ResetPiece(&player);

    float timer = 0;
    float dropInterval = 0.5f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float currentInterval = IsKeyDown(KEY_S) ? 0.05f : dropInterval;

        if (IsKeyPressed(KEY_LEFT) && !CheckCollision(player.x - 1, player.y, player.type.matrix))
            player.x--;
        if (IsKeyPressed(KEY_RIGHT) && !CheckCollision(player.x + 1, player.y, player.type.matrix))
            player.x++;
        if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
            RotatePiece(&player);

        timer += dt;
        if (timer >= currentInterval)
        {
            if (!CheckCollision(player.x, player.y + 1, player.type.matrix))
            {
                player.y++;
            }
            else
            {
                for (int r = 0; r < 4; r++)
                {
                    for (int c = 0; c < 4; c++)
                    {
                        if (player.type.matrix[r][c] && (player.y + r) >= 0)
                        {
                            grid[player.y + r][player.x + c] = player.type.id;
                            gridColors[player.y + r][player.x + c] = player.type.color;
                        }
                    }
                }
                ClearLines();
                ResetPiece(&player);
            }
            timer = 0;
        }

        BeginDrawing();
        ClearBackground((Color){10, 10, 12, 255});

        //绘制游戏区
        for (int i = 0; i <= COLS; i++)
            DrawLine(i * CELL_SIZE, 0, i * CELL_SIZE, GetScreenHeight(), Fade(GRAY, 0.1f));
        for (int r = 0; r < ROWS; r++)
            for (int c = 0; c < COLS; c++)
                if (grid[r][c])
                    DrawFancyBlock(c, r, gridColors[r][c], CELL_SIZE);
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                if (player.type.matrix[r][c])
                    DrawFancyBlock(player.x + c, player.y + r, player.type.color, CELL_SIZE);

        //绘制侧边栏 UI
        int uiX = COLS * CELL_SIZE + 30;
        DrawRectangle(COLS * CELL_SIZE, 0, sidebarWidth, GetScreenHeight(), (Color){25, 25, 30, 255});

        // 标题
        DrawText("TETRIS", uiX, 40, 45, GOLD);
        DrawLine(uiX, 95, uiX + 220, 95, DARKGRAY);

        // 分数显示
        char scoreText[20], lineText[20];
        sprintf(scoreText, "%06d", score);
        sprintf(lineText, "LINES: %d", linesCleared);

        DrawText("SCORE", uiX, 130, 25, SKYBLUE);
        DrawText(scoreText, uiX, 165, 40, RAYWHITE);

        DrawText(lineText, uiX, 230, 25, LIGHTGRAY);

        // 下一个方块预览区
        DrawText("NEXT", uiX, 320, 25, ORANGE);
        DrawRectangle(uiX, 360, 160, 160, (Color){15, 15, 18, 255});
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (nextPiece.matrix[r][c])
                {
                    // 在预览窗口居中绘制
                    DrawFancyBlock((uiX / 30) + c + 8, (370 / 30) + r + 1, nextPiece.color, 30);
                }
            }
        }

        // 操作指南
        DrawText("CONTROLS", uiX, 600, 20, GRAY);
        DrawText("ARROWS : Move", uiX, 630, 18, DARKGRAY);
        DrawText("W / UP : Rotate", uiX, 655, 18, DARKGRAY);
        DrawText("S      : Fast", uiX, 680, 18, DARKGRAY);

        DrawFPS(uiX, GetScreenHeight() - 40);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}