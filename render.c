#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "./game.h"
#include "./render.h"
#include "./logic.h"


void receive_first_row(board_t *board, unsigned char **pString, int neighborRank);
void send_last_row(board_t *board, unsigned char **pString, int neighborRank);

void render_board(board_t* board, unsigned char** neighbors, int neighborRank)
{
    count_neighbors(board, neighbors);
    return;
    switch(board->game_state) {
        case RUNNING_STATE:
            //print_board(board);
            count_neighbors(board, neighbors);
            send_last_row(board, neighbors, neighborRank);
            receive_first_row(board, neighbors, neighborRank);
            evolve(board, neighbors);
            break;
        case PAUSE_STATE:
            break;
        default: {}
    }
}


void print_board(board_t* board)
{
    printf("\n");
    for (int i = 0; i < board->ROW_NUM; i++) {
        for (int j = 0; j < board->COL_NUM; j++) {
            printf("%d ", board->cell_state[i][j]);
        }
        printf("\n");
    }
}

