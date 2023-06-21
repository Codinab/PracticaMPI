#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "./game.h"
#include "./render.h"
#include "./logic.h"


void send_rows(board_t *board, unsigned char **pString, int previousRank, int nextRank, MPI_Request *requests,
               int *request_count);

void receive_rows(board_t *board, unsigned char **pString, int previousRank, int nextRank, MPI_Request *requests,
                  int *request_count);

void render_board(board_t *board, unsigned char **neighbors, int previousRank, int nextRank) {
    MPI_Request requests[4];
    MPI_Status statuses[4];
    int request_count = 0;

    switch (board->game_state) {
        case RUNNING_STATE:
            //print_board(board);
            count_neighbors(board, neighbors);
            receive_rows(board, neighbors, previousRank, nextRank, requests, &request_count);
            send_rows(board, neighbors, previousRank, nextRank, requests, &request_count);
            MPI_Waitall(request_count, requests, statuses);
            evolve(board, neighbors);
            break;
        case PAUSE_STATE:
            break;
        default: {
        }
    }
}


void print_board(board_t *board) {
    printf("\n");
    for (int i = 0; i < board->ROW_NUM; i++) {
        for (int j = 0; j < board->COL_NUM; j++) {
            printf("%d ", board->cell_state[i][j]);
        }
        printf("\n");
    }
}

