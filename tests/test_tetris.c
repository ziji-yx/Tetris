#include <assert.h>
#include <string.h>

#define main tetris_application_main
#include "../Tetris.demo.c"
#undef main

static void TestCollisionBoundaries(void)
{
    int block[4][4] = {{1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

    memset(board, 0, sizeof(board));

    assert(!CheckCollision(0, 0, block));
    assert(!CheckCollision(COLS - 1, ROWS - 1, block));
    assert(CheckCollision(-1, 0, block));
    assert(CheckCollision(COLS, 0, block));
    assert(CheckCollision(0, ROWS, block));

    board[ROWS - 1][0] = 1;
    assert(CheckCollision(0, ROWS - 1, block));
}

static void TestOShapeDoesNotRotate(void)
{
    int before[4][4];

    activePiece.shape = SHAPES[PIECE_O];
    activePiece.x = 3;
    activePiece.y = 0;
    memcpy(before, activePiece.shape.matrix, sizeof(before));

    TryRotatePiece();

    assert(memcmp(before, activePiece.shape.matrix, sizeof(before)) == 0);
    assert(activePiece.x == 3);
    assert(activePiece.y == 0);
}

static void TestSingleLineClear(void)
{
    memset(board, 0, sizeof(board));
    memset(boardColors, 0, sizeof(boardColors));

    for (int c = 0; c < COLS; c++)
    {
        board[ROWS - 1][c] = 1;
    }

    board[ROWS - 2][0] = 2;

    assert(ClearFullLines() == 1);
    assert(board[ROWS - 1][0] == 2);
    assert(board[0][0] == 0);
}

static void TestFourLineClear(void)
{
    memset(board, 0, sizeof(board));
    memset(boardColors, 0, sizeof(boardColors));

    for (int r = ROWS - 4; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            board[r][c] = 1;
        }
    }

    assert(ClearFullLines() == 4);

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            assert(board[r][c] == 0);
        }
    }
}

static void TestSevenBagContainsEveryPiece(void)
{
    for (int round = 0; round < 20; round++)
    {
        int counts[PIECE_COUNT] = {0};

        RefillBag();

        for (int i = 0; i < PIECE_COUNT; i++)
        {
            counts[(int)bag[i]]++;
        }

        for (int i = 0; i < PIECE_COUNT; i++)
        {
            assert(counts[i] == 1);
        }
    }
}

int main(void)
{
    TestCollisionBoundaries();
    TestOShapeDoesNotRotate();
    TestSingleLineClear();
    TestFourLineClear();
    TestSevenBagContainsEveryPiece();
    return 0;
}
