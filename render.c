#include "./game.h"
#include "./render.h"
#include "./logic.h"


void render_board(board_t* board, unsigned char neighbors[D_COL_NUM][D_ROW_NUM])
{
    switch(board->game_state) {
        case RUNNING_STATE:
            count_neighbors(board, neighbors);
            evolve(board, neighbors);
            break;
        case PAUSE_STATE:
            break;
        default: {}
    }
}
