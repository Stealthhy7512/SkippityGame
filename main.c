#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


typedef struct {
    int size;
    char** data;
} Board;

typedef struct {
    int score;
    int undo;
    int skippies[5];
} Player;

Board createBoard(const int size)
{
    Board board;
    int i;

    board.size = size;
    board.data = calloc(size, sizeof(char*));

    for (i = 0; i < size; ++i)
        board.data[i] = calloc(size, sizeof(char));

    return board;
}

void setBoard(Board* board)
{
    int i, j;

    for (i = 0; i < board->size; ++i)
        for (j = 0; j < board->size; ++j)
            board->data[i][j] = 'a' + (rand() % 5);

    board->data[board->size / 2 - 1][board->size / 2 - 1] = ' ';
    board->data[board->size / 2 - 1][board->size / 2] = ' ';
    board->data[board->size / 2][board->size / 2 - 1] = ' ';
    board->data[board->size / 2][board->size / 2] = ' ';
}

void printBoard(const Board board)
{
    int i, j;

    for (i = 0; i < board.size; ++i) {
        for (j = 0; j < board.size; ++j)
            printf("%c ", board.data[i][j]);
        puts("");
    }
}

void moveSkipper(Board* board, Player* player)
{
    int x0, y0, x1, y1;
    fputs("Enter the coordinates for the skipper you wish to move: ", stdout);
    scanf("%d %d", &x0, &y0);

    fputs("Enter the coordinates for where you want to move: ", stdout);
    scanf("%d %d", &x1, &y1);

    if ((x1 - x0 != 2) && (y1 - y0 != 2)) {
        fputs("Diagonal move, illegal.", stderr);
        return;
    }

    else {
        board->data[x1][y1] = board->data[x0][y0];
        board->data[x0][y0] = ' ';
        player->skippies[board->data[(x1 + x0) / 2][(y1 + y0) / 2] - 'a']++;
        board->data[(x1 + x0) / 2][(y1 + y0) / 2] = ' ';
    }
}

int main(void)
{
    srand(time(NULL));
    Board board = createBoard(10);
    Player p1 = { 0, 0, {0, 0, 0, 0, 0} }, p2 = { 0, 0, {0, 0, 0, 0, 0} };
    setBoard(&board);
    printBoard(board);
    moveSkipper(&board, &p1);
    printBoard(board);
    int i;
    for (i = 0; i < 5; ++i)
        printf("%d ", p1.skippies[i]);
}
