#ifndef GAME_H_
#define GAME_H_

#define D_COL_NUM 4000 // size of game universe, only windowed is shown.
#define D_ROW_NUM 4000
#define ALIVE 1
#define DEAD 0

#define RUNNING_STATE 0
#define PAUSE_STATE 1

//#define GRAPHICAL_MODE

typedef struct {
  unsigned char **cell_state;
  unsigned char *ghost_cell_state[2];
  int game_state;
  int COL_NUM;
  int ROW_NUM;
} board_t;

void allocate_board(board_t* board);

void free_board(board_t* board);

void init_board(board_t *board);

#endif // GAME_H_
