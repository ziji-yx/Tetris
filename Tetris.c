#include <raylib.h>
#include <time.h>

#define BLOCK_SIZE 50
#define GRID_WIDTH 10
#define GRID_LENGTH 20
#define SCREEN_WIDTH (BLOCK_SIZE * GRID_WIDTH)
#define SCREEN_HEIGHT (BLOCK_SIZE * GRID_LENGTH + 100)

typedef enum
{
	MENU,
	SELECT_SPEED,
	PLAYING,
	INPUT_NAME,
	SHOW_RANK,
	GAME_OVER
} State;

typedef struct
{
	int id;
	Color color;
	int matrix[4][4];
} Tetromino;

typedef struct
{
	int x, y;
	Tetromino shape;
} Player;

Tetromino shapes[7] = {
	{1, {0, 121, 241, 255}, {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}},	 // I (SkyBlue)
	{2, {0, 82, 172, 255}, {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},	 // J (Blue)
	{3, {255, 161, 0, 255}, {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},	 // L (Orange)
	{4, {253, 249, 0, 255}, {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},	 // O (Yellow)
	{5, {0, 228, 48, 255}, {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},	 // S (Green)
	{6, {200, 122, 255, 255}, {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}}, // T (Purple)
	{7, {230, 41, 55, 255}, {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}}	 // Z (Red)
};

int main(void)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tetris");
	SetTraceLogLevel(LOG_WARNING);
	SetTargetFPS(60);
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(WHITE);
		EndDrawing();
	}
	CloseWindow();
}

void WaitTime(double second);
bool IsKeyPressed(int key); // 检查是否按下了一次键
bool IsKeyDown(int key);	// 检查是否按下了键
DrawRectangle();