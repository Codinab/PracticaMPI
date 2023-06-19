#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>


#ifndef NO_SDL
#include <SDL2/SDL.h>
#endif

#include "./game.h"
#include "./logic.h"
#include "./render.h"



void usage()
{
  printf("\nUsage: conway [-w weight] [-h height] [-i input_board_file] [-o output_board_file] [-e End_time] [-c cell_size] \n\t");
  printf("\n -g\tEnable graphical mode.\n\n");
  printf("\n -w\tSet board weight.\n\n");
  printf("\n -h\tSet board height.\n\n");
  printf("\n -i\tInput board file.\n\n");
  printf("\n -o\tOutput board file.\n\n");
  printf("\n -e\tNumber of simulation iterations.\n\n");
  printf("Enter extremely low values at own peril.\n\tRecommended to stay in 30000-100000 range.\n\tDefaults to 50000.\n\n");
  printf("\n -c\tSet cell size to tiny, small, medium or large.\n\tDefaults to small.\n\n");
}

int main(int argc, char** argv)
{
  bool LoadFile = false, SaveFile = false;
  int EndTime=-1;
  char input_file[256],output_file[256];

  board_t *board = (board_t*) malloc(sizeof(board_t));
  if (board==NULL) {
  	fprintf(stderr,"Error reserving board memory %lf KB",sizeof(board_t)/1024.0);
  	exit(1);
  }
  // Configure board initial state.
  board->game_state = RUNNING_STATE;
  board->CELL_WIDTH = 4; // Reasonable default size
  board->CELL_HEIGHT = 4;
  board->COL_NUM = D_COL_NUM;
  board->ROW_NUM = D_ROW_NUM;

  for (int i = 0; i < board->COL_NUM; i++) {
    for (int j = 0; j < board->ROW_NUM; j++)
      board->cell_state[i][j] = DEAD;
  }

  unsigned char neighbors[D_COL_NUM][D_ROW_NUM] = {DEAD};

  // Command line options.
  int opt;

  while((opt = getopt(argc, argv, "c:h:i:o:w:H:e")) != -1) {
    switch (opt) {
      case 'i':
      	strcpy(input_file,optarg);
      	LoadFile = true;
      	break;
      case 'o':
      	SaveFile=true;
      	strcpy(output_file,optarg);
      	printf("Output Board file %s.\n",optarg);
      	break;
      case 'w':
        board->COL_NUM = atoi(optarg);
        printf("Board width %d.\n",board->COL_NUM);
        break;
      case 'h':
        board->ROW_NUM = atoi(optarg);
        printf("Board height %d.\n",board->ROW_NUM);
        break;
      case 'e':
      	printf("End Time: %s.\n",optarg);
        EndTime = atoi(optarg);
        break;
      case 'c':
        if (strcmp(optarg,"tiny") == 0) {
          board->CELL_WIDTH = 2;
          board->CELL_HEIGHT = 2;
        }
        else if (strcmp(optarg,"small") == 0) {
          board->CELL_WIDTH = 5;
          board->CELL_HEIGHT = 5;
        }
        else if (strcmp(optarg,"medium") == 0) {
          board->CELL_WIDTH = 10;
          board->CELL_HEIGHT = 10;
        }
        else if (strcmp(optarg,"large") == 0) {
          board->CELL_WIDTH = 25;
          board->CELL_HEIGHT = 25;
        }
        break;
      case 'H':
        usage();
        exit(EXIT_SUCCESS);
        break;
      case '?':
        if (optopt == 't' || optopt == 's' || optopt == 'c' || optopt == 'i' || optopt == 'o' || optopt == 'w' || optopt == 'h' || optopt == 'e' )
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        printf("Setting default options.\n");
        usage();
        break;
      default:
        printf("Setting default options.\n");
        usage();
        break;
    }
  }

  if (LoadFile)
  {
  	printf("Loading Board file %s.\n",input_file);
    life_read(input_file, board);
  }
  else
  { // Rando, init file
  	printf("Init Cells\n");fflush(stdout);
    double prob = 0.94;
  	int seed = 123456789;
  	life_init(board, prob, &seed);
  }

  printf("Start Simulation.\n");fflush(stdout);
  int Iteration=0;
  while ((EndTime < 0 || Iteration < EndTime) != 0)
  {
    render_board(board, neighbors);
    printf("[%05d] Life Game Simulation step.\r",++Iteration); fflush(stdout);
  }
  printf("\nEnd Simulation.\n");

  // Save board
  if (SaveFile) {
	 printf("Writing Board file %s.\n",output_file); fflush(stdout);
     life_write(output_file, board);
  }

  return EXIT_SUCCESS;
}
