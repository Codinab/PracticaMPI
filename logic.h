#include <mpi.h>

void count_neighbors(board_t* board, unsigned char** neighbors,int *request_count , MPI_Request *requests,
                     MPI_Status *statuses);

void count_neighbors_spherical_world(board_t* board, unsigned char** neighbors);

void count_neighbors_flat_world(board_t* board, unsigned char** neighbors);

void count_neighbors_toroidal_world(board_t *board, unsigned char **neighbors, const int *request_count, MPI_Request *requests,
                                    MPI_Status *statuses);

void evolve(board_t* board, unsigned char** neighbors);

void life_read ( char *filename, board_t* board);

void life_write ( char *output_filename, board_t* board);

double r8_uniform_01 ( int *seed );

void life_init (board_t* board, double prob, int *seed );
