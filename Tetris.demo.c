#include "raylib.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define COLS 10
#define ROWS 20
#define CELL_SIZE 40
#define BOARD_WIDTH (COLS * CELL_SIZE)
#define BOARD_HEIGHT (ROWS * CELL_SIZE)
#define SIDEBAR_WIDTH 400
#define NEXT_QUEUE_SIZE 3
#define PIECE_COUNT 7
#define LOCK_DELAY 0.28f
#define SOFT_DROP_INTERVAL 0.045f

typedef enum
{
    PIECE_I = 0,
    PIECE_J,
    PIECE_L,
    PIECE_O,
    PIECE_S,
    PIECE_T,
    PIECE_Z
} PieceType;

typedef struct
{
    PieceType type;
    int matrix[4][4];
    Color color;
} Tetromino;

typedef struct
{
    int x;
    int y;
    Tetromino shape;
} ActivePiece;

typedef enum
{
    GAME_PLAYING,
    GAME_PAUSED,
    GAME_OVER
} GameState;

static const Tetromino SHAPES[PIECE_COUNT] = {
    {PIECE_I, {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}, {0, 188, 212, 255}},
    {PIECE_J, {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, {46, 104, 255, 255}},
    {PIECE_L, {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, {255, 159, 28, 255}},
    {PIECE_O, {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, {255, 214, 64, 255}},
    {PIECE_S, {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, {76, 201, 76, 255}},
    {PIECE_T, {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, {181, 102, 255, 255}},
    {PIECE_Z, {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, {239, 71, 111, 255}}
};

static int board[ROWS][COLS];
static Color boardColors[ROWS][COLS];
static ActivePiece activePiece;
static PieceType nextQueue[NEXT_QUEUE_SIZE];
static PieceType bag[PIECE_COUNT];
static int bagIndex = PIECE_COUNT;
static int heldType = -1;
static bool hasHeldPiece = false;
static bool holdUsed = false;
static GameState gameState = GAME_PLAYING;
static int score = 0;
static int highScore = 0;
static int linesCleared = 0;
static int level = 1;
static float dropTimer = 0.0f;
static float lockTimer = 0.0f;

static int LoadHighScore(void)
{
    int value = 0;
    FILE *file = fopen("record.txt", "r");

    if (file != NULL)
    {
        if (fscanf(file, "%d", &value) != 1 || value < 0)
        {
            value = 0;
        }
        fclose(file);
    }

    return value;
}

static void SaveHighScore(void)
{
    FILE *file = fopen("record.txt", "w");

    if (file != NULL)
    {
        fprintf(file, "%d\n", highScore);
        fclose(file);
    }
}

static bool CheckCollision(int nx, int ny, int matrix[4][4])
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (!matrix[r][c])
            {
                continue;
            }

            int gx = nx + c;
            int gy = ny + r;

            if (gx < 0 || gx >= COLS || gy >= ROWS)
            {
                return true;
            }

            if (gy >= 0 && board[gy][gx])
            {
                return true;
            }
        }
    }

    return false;
}

static void RefillBag(void)
{
    for (int i = 0; i < PIECE_COUNT; i++)
    {
        bag[i] = (PieceType)i;
    }

    for (int i = PIECE_COUNT - 1; i > 0; i--)
    {
        int j = GetRandomValue(0, i);
        PieceType temp = bag[i];
        bag[i] = bag[j];
        bag[j] = temp;
    }

    bagIndex = 0;
}

static PieceType NextPieceType(void)
{
    if (bagIndex >= PIECE_COUNT)
    {
        RefillBag();
    }

    return bag[bagIndex++];
}

static float GravityInterval(void)
{
    float interval = 0.55f - (float)(level - 1) * 0.045f;

    if (interval < 0.08f)
    {
        interval = 0.08f;
    }

    return interval;
}

static void SpawnPiece(PieceType type)
{
    activePiece.shape = SHAPES[type];
    activePiece.x = COLS / 2 - 2;
    activePiece.y = 0;
    dropTimer = 0.0f;
    lockTimer = 0.0f;
}

static void SetGameOver(void)
{
    gameState = GAME_OVER;

    if (score > highScore)
    {
        highScore = score;
    }

    SaveHighScore();
}

static void SpawnNextPiece(void)
{
    PieceType type = nextQueue[0];

    for (int i = 1; i < NEXT_QUEUE_SIZE; i++)
    {
        nextQueue[i - 1] = nextQueue[i];
    }

    nextQueue[NEXT_QUEUE_SIZE - 1] = NextPieceType();
    SpawnPiece(type);
    holdUsed = false;

    if (CheckCollision(activePiece.x, activePiece.y, activePiece.shape.matrix))
    {
        SetGameOver();
    }
}

static void ResetGame(void)
{
    memset(board, 0, sizeof(board));
    memset(boardColors, 0, sizeof(boardColors));

    score = 0;
    linesCleared = 0;
    level = 1;
    heldType = -1;
    hasHeldPiece = false;
    holdUsed = false;
    bagIndex = PIECE_COUNT;
    dropTimer = 0.0f;
    lockTimer = 0.0f;
    gameState = GAME_PLAYING;

    for (int i = 0; i < NEXT_QUEUE_SIZE; i++)
    {
        nextQueue[i] = NextPieceType();
    }

    SpawnNextPiece();
}

static bool TryMove(int dx, int dy)
{
    if (CheckCollision(activePiece.x + dx, activePiece.y + dy, activePiece.shape.matrix))
    {
        return false;
    }

    activePiece.x += dx;
    activePiece.y += dy;
    return true;
}

static void RotateMatrixClockwise(int source[4][4], int target[4][4], int size)
{
    memset(target, 0, sizeof(int) * 16);

    for (int r = 0; r < size; r++)
    {
        for (int c = 0; c < size; c++)
        {
            target[c][size - 1 - r] = source[r][c];
        }
    }
}

static void TryRotatePiece(void)
{
    if (activePiece.shape.type == PIECE_O)
    {
        return;
    }

    int rotated[4][4];
    int size = (activePiece.shape.type == PIECE_I) ? 4 : 3;
    static const int kickX[] = {0, -1, 1, -2, 2, 0, -1, 1};
    static const int kickY[] = {0, 0, 0, 0, 0, -1, -1, -1};

    RotateMatrixClockwise(activePiece.shape.matrix, rotated, size);

    for (int i = 0; i < (int)(sizeof(kickX) / sizeof(kickX[0])); i++)
    {
        int nx = activePiece.x + kickX[i];
        int ny = activePiece.y + kickY[i];

        if (!CheckCollision(nx, ny, rotated))
        {
            activePiece.x = nx;
            activePiece.y = ny;
            memcpy(activePiece.shape.matrix, rotated, sizeof(rotated));
            lockTimer = 0.0f;
            return;
        }
    }
}

static int ClearFullLines(void)
{
    int cleared = 0;

    for (int r = ROWS - 1; r >= 0; r--)
    {
        bool full = true;

        for (int c = 0; c < COLS; c++)
        {
            if (!board[r][c])
            {
                full = false;
                break;
            }
        }

        if (!full)
        {
            continue;
        }

        for (int row = r; row > 0; row--)
        {
            memcpy(board[row], board[row - 1], sizeof(board[row]));
            memcpy(boardColors[row], boardColors[row - 1], sizeof(boardColors[row]));
        }

        memset(board[0], 0, sizeof(board[0]));
        memset(boardColors[0], 0, sizeof(boardColors[0]));
        cleared++;
        r++;
    }

    return cleared;
}

static void LockActivePiece(void)
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (!activePiece.shape.matrix[r][c])
            {
                continue;
            }

            int gx = activePiece.x + c;
            int gy = activePiece.y + r;

            if (gy >= 0 && gy < ROWS && gx >= 0 && gx < COLS)
            {
                board[gy][gx] = (int)activePiece.shape.type + 1;
                boardColors[gy][gx] = activePiece.shape.color;
            }
        }
    }

    int cleared = ClearFullLines();

    if (cleared > 0)
    {
        static const int lineScores[] = {0, 100, 300, 500, 800};
        score += lineScores[cleared] * level;
        linesCleared += cleared;
        level = 1 + linesCleared / 10;
    }

    if (score > highScore)
    {
        highScore = score;
    }

    SpawnNextPiece();
}

static void HardDrop(void)
{
    int distance = 0;

    while (!CheckCollision(activePiece.x, activePiece.y + distance + 1, activePiece.shape.matrix))
    {
        distance++;
    }

    activePiece.y += distance;
    score += distance * 2;
    LockActivePiece();
}

static void HoldCurrentPiece(void)
{
    if (holdUsed || gameState != GAME_PLAYING)
    {
        return;
    }

    if (hasHeldPiece)
    {
        int swapType = heldType;
        heldType = (int)activePiece.shape.type;
        SpawnPiece((PieceType)swapType);

        if (CheckCollision(activePiece.x, activePiece.y, activePiece.shape.matrix))
        {
            SetGameOver();
            return;
        }
    }
    else
    {
        heldType = (int)activePiece.shape.type;
        hasHeldPiece = true;
        SpawnNextPiece();

        if (gameState != GAME_PLAYING)
        {
            return;
        }
    }

    holdUsed = true;
}

static void UpdatePlaying(float dt)
{
    if (IsKeyPressed(KEY_P))
    {
        gameState = GAME_PAUSED;
        return;
    }

    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT))
    {
        HoldCurrentPiece();

        if (gameState != GAME_PLAYING)
        {
            return;
        }
    }

    if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && TryMove(-1, 0))
    {
        lockTimer = 0.0f;
    }

    if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && TryMove(1, 0))
    {
        lockTimer = 0.0f;
    }

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    {
        TryRotatePiece();
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        HardDrop();
        return;
    }

    bool softDrop = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
    float interval = softDrop ? SOFT_DROP_INTERVAL : GravityInterval();
    dropTimer += dt;

    while (dropTimer >= interval && gameState == GAME_PLAYING)
    {
        dropTimer -= interval;

        if (TryMove(0, 1))
        {
            lockTimer = 0.0f;

            if (softDrop)
            {
                score++;
            }
        }
        else
        {
            dropTimer = 0.0f;
            break;
        }
    }

    if (CheckCollision(activePiece.x, activePiece.y + 1, activePiece.shape.matrix))
    {
        lockTimer += dt;

        if (lockTimer >= LOCK_DELAY)
        {
            LockActivePiece();
        }
    }
    else
    {
        lockTimer = 0.0f;
    }

    if (score > highScore)
    {
        highScore = score;
    }
}

static void UpdateGame(float dt)
{
    if (IsKeyPressed(KEY_R))
    {
        ResetGame();
        return;
    }

    switch (gameState)
    {
        case GAME_PLAYING:
            UpdatePlaying(dt);
            break;

        case GAME_PAUSED:
            if (IsKeyPressed(KEY_P))
            {
                gameState = GAME_PLAYING;
            }
            break;

        case GAME_OVER:
            if (IsKeyPressed(KEY_ENTER))
            {
                ResetGame();
            }
            break;
    }
}

static void DrawBlock(int x, int y, int size, Color color)
{
    Color top = ColorBrightness(color, 0.28f);

    DrawRectangleGradientV(x + 1, y + 1, size - 2, size - 2, top, color);
    DrawRectangleLines(x + 1, y + 1, size - 2, size - 2, Fade(WHITE, 0.26f));
    DrawRectangle(x + 5, y + 5, size / 6, size / 6, Fade(WHITE, 0.42f));
}

static int GhostY(void)
{
    int y = activePiece.y;

    while (!CheckCollision(activePiece.x, y + 1, activePiece.shape.matrix))
    {
        y++;
    }

    return y;
}

static void DrawBoard(void)
{
    int ghostY = GhostY();

    DrawRectangle(0, 0, BOARD_WIDTH, BOARD_HEIGHT, (Color){12, 15, 21, 255});

    for (int c = 0; c <= COLS; c++)
    {
        DrawLine(c * CELL_SIZE, 0, c * CELL_SIZE, BOARD_HEIGHT, Fade(RAYWHITE, 0.08f));
    }

    for (int r = 0; r <= ROWS; r++)
    {
        DrawLine(0, r * CELL_SIZE, BOARD_WIDTH, r * CELL_SIZE, Fade(RAYWHITE, 0.08f));
    }

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (board[r][c])
            {
                DrawBlock(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, boardColors[r][c]);
            }
        }
    }

    BeginScissorMode(0, 0, BOARD_WIDTH, BOARD_HEIGHT);

    if (ghostY > activePiece.y)
    {
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (!activePiece.shape.matrix[r][c])
                {
                    continue;
                }

                int x = (activePiece.x + c) * CELL_SIZE;
                int y = (ghostY + r) * CELL_SIZE;
                DrawRectangleLinesEx(
                    (Rectangle){(float)x + 2.0f, (float)y + 2.0f, (float)CELL_SIZE - 4.0f, (float)CELL_SIZE - 4.0f},
                    2.0f,
                    Fade(activePiece.shape.color, 0.55f));
            }
        }
    }

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (activePiece.shape.matrix[r][c])
            {
                DrawBlock(
                    (activePiece.x + c) * CELL_SIZE,
                    (activePiece.y + r) * CELL_SIZE,
                    CELL_SIZE,
                    activePiece.shape.color);
            }
        }
    }

    EndScissorMode();
    DrawRectangleLinesEx((Rectangle){0, 0, BOARD_WIDTH, BOARD_HEIGHT}, 2.0f, Fade(RAYWHITE, 0.24f));
}

static void DrawPiecePreview(const Tetromino *shape, Rectangle box)
{
    int minRow = 4;
    int maxRow = -1;
    int minCol = 4;
    int maxCol = -1;

    DrawRectangleRec(box, (Color){15, 18, 25, 255});
    DrawRectangleLinesEx(box, 2.0f, Fade(RAYWHITE, 0.15f));

    if (shape == NULL)
    {
        const char *empty = "EMPTY";
        int width = MeasureText(empty, 14);
        DrawText(empty, (int)(box.x + box.width / 2.0f) - width / 2, (int)(box.y + box.height / 2.0f) - 7, 14, DARKGRAY);
        return;
    }

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (!shape->matrix[r][c])
            {
                continue;
            }

            if (r < minRow) minRow = r;
            if (r > maxRow) maxRow = r;
            if (c < minCol) minCol = c;
            if (c > maxCol) maxCol = c;
        }
    }

    if (maxRow < 0)
    {
        return;
    }

    const int blockSize = 25;
    int pieceWidth = (maxCol - minCol + 1) * blockSize;
    int pieceHeight = (maxRow - minRow + 1) * blockSize;
    int startX = (int)(box.x + (box.width - (float)pieceWidth) / 2.0f) - minCol * blockSize;
    int startY = (int)(box.y + (box.height - (float)pieceHeight) / 2.0f) - minRow * blockSize;

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (shape->matrix[r][c])
            {
                DrawBlock(startX + c * blockSize, startY + r * blockSize, blockSize, shape->color);
            }
        }
    }
}

static void DrawSidebar(void)
{
    const int boardX = BOARD_WIDTH;
    const int uiX = boardX + 34;
    const int previewX = boardX + 115;
    const int previewWidth = 170;
    const int previewHeight = 142;
    char scoreText[32];
    char bestText[32];
    char levelText[32];
    char linesText[32];

    DrawRectangle(boardX, 0, SIDEBAR_WIDTH, BOARD_HEIGHT, (Color){24, 28, 37, 255});
    DrawRectangle(boardX, 0, 5, BOARD_HEIGHT, (Color){255, 199, 61, 255});

    DrawText("TETRIS", uiX, 25, 48, GOLD);
    DrawText("C / RAYLIB", uiX + 2, 78, 14, DARKGRAY);
    DrawFPS(BOARD_WIDTH + SIDEBAR_WIDTH - 80, 28);

    snprintf(scoreText, sizeof(scoreText), "%07d", score);
    snprintf(bestText, sizeof(bestText), "%07d", highScore);
    snprintf(levelText, sizeof(levelText), "%02d", level);
    snprintf(linesText, sizeof(linesText), "%03d", linesCleared);

    DrawText("SCORE", uiX, 112, 15, GRAY);
    DrawText(scoreText, uiX, 134, 34, RAYWHITE);
    DrawText("BEST", uiX, 184, 15, GRAY);
    DrawText(bestText, uiX, 206, 27, (Color){255, 199, 61, 255});

    DrawText("LEVEL", uiX, 254, 15, GRAY);
    DrawText(levelText, uiX, 275, 28, SKYBLUE);
    DrawText("LINES", uiX + 145, 254, 15, GRAY);
    DrawText(linesText, uiX + 145, 275, 28, LIGHTGRAY);

    DrawText("NEXT", uiX, 326, 15, GRAY);
    DrawPiecePreview(&SHAPES[nextQueue[0]], (Rectangle){(float)previewX, 351.0f, (float)previewWidth, (float)previewHeight});

    DrawText("HOLD", uiX, 518, 15, GRAY);
    DrawPiecePreview(
        hasHeldPiece ? &SHAPES[heldType] : NULL,
        (Rectangle){(float)previewX, 543.0f, (float)previewWidth, (float)previewHeight});

    DrawText("CONTROLS", uiX, 708, 13, GRAY);
    DrawText("MOVE    ARROWS / A D", uiX, 729, 12, DARKGRAY);
    DrawText("ROTATE  W / UP", uiX, 746, 12, DARKGRAY);
    DrawText("SOFT    S / DOWN", uiX, 763, 12, DARKGRAY);
    DrawText("HARD    SPACE", uiX + 205, 729, 12, DARKGRAY);
    DrawText("HOLD    C / SHIFT", uiX + 205, 746, 12, DARKGRAY);
    DrawText("PAUSE   P", uiX + 205, 763, 12, DARKGRAY);
}

static void DrawCenteredBoardText(const char *text, int y, int size, Color color)
{
    int width = MeasureText(text, size);
    DrawText(text, (BOARD_WIDTH - width) / 2, y, size, color);
}

static void DrawGameOverlay(void)
{
    if (gameState == GAME_PLAYING)
    {
        return;
    }

    DrawRectangle(0, 0, BOARD_WIDTH, BOARD_HEIGHT, Fade(BLACK, 0.76f));
    DrawRectangleLinesEx(
        (Rectangle){70.0f, 250.0f, BOARD_WIDTH - 140.0f, 280.0f},
        2.0f,
        Fade(RAYWHITE, 0.2f));

    if (gameState == GAME_PAUSED)
    {
        DrawCenteredBoardText("PAUSED", 310, 40, WHITE);
        DrawCenteredBoardText("PRESS P TO CONTINUE", 370, 16, LIGHTGRAY);
        DrawCenteredBoardText("PRESS R TO RESTART", 398, 16, DARKGRAY);
        return;
    }

    char scoreText[64];
    char bestText[64];
    snprintf(scoreText, sizeof(scoreText), "SCORE  %d", score);
    snprintf(bestText, sizeof(bestText), "BEST   %d", highScore);

    DrawCenteredBoardText("GAME OVER", 290, 38, (Color){255, 91, 105, 255});
    DrawCenteredBoardText(scoreText, 352, 20, RAYWHITE);
    DrawCenteredBoardText(bestText, 382, 18, GOLD);
    DrawCenteredBoardText("PRESS R OR ENTER TO RESTART", 455, 15, LIGHTGRAY);
}

static void DrawGame(void)
{
    BeginDrawing();
    ClearBackground((Color){8, 10, 15, 255});

    DrawBoard();
    DrawSidebar();
    DrawGameOverlay();

    EndDrawing();
}

int main(void)
{
    InitWindow(BOARD_WIDTH + SIDEBAR_WIDTH, BOARD_HEIGHT, "Tetris - C / raylib");
    SetTargetFPS(60);

    highScore = LoadHighScore();
    ResetGame();

    while (!WindowShouldClose())
    {
        UpdateGame(GetFrameTime());
        DrawGame();
    }

    if (score > highScore)
    {
        highScore = score;
    }

    SaveHighScore();
    CloseWindow();
    return 0;
}
