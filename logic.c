#include <stdio.h>
#include "./game.h"
#include "./logic.h"
#include "./render.h"

#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>

void count_neighbors(board_t *board, unsigned char **neighbors, int *request_count, MPI_Request *requests,
                     MPI_Status *statuses) {
    count_neighbors_toroidal_world(board, neighbors, request_count, requests, statuses);
}

void send_rows(board_t *board, unsigned char **pString, int previousRank, int nextRank, MPI_Request *requests,
               int *request_count) {

    // Initiate sending the first row to the previous rank
    MPI_Isend(board->cell_state[0], board->COL_NUM, MPI_UNSIGNED_CHAR, previousRank, 1, MPI_COMM_WORLD,
              &requests[(*request_count)++]);

    // Initiate sending the last row to the next rank
    MPI_Isend(board->cell_state[board->ROW_NUM - 1], board->COL_NUM, MPI_UNSIGNED_CHAR, nextRank, 2, MPI_COMM_WORLD,
              &requests[(*request_count)++]);
}

void receive_rows(board_t *board, unsigned char **pString, int previousRank, int nextRank, MPI_Request *requests,
                  int *request_count) {

    // Initiate receiving the previous rank's last row
    MPI_Irecv(board->ghost_cell_state[0], board->COL_NUM, MPI_UNSIGNED_CHAR, previousRank, 2, MPI_COMM_WORLD,
              &requests[(*request_count)++]);

    // Initiate receiving the next rank's first row
    MPI_Irecv(board->ghost_cell_state[1], board->COL_NUM, MPI_UNSIGNED_CHAR, nextRank, 1, MPI_COMM_WORLD, &requests[(*request_count)++]);
}


void count_neighbors_toroidal_world(board_t *board, unsigned char **neighbors) {
    int i_prev, i_next, j_prev, j_next;

    // Clear neighbors
    for (int i = 0; i < board->ROW_NUM; i++) {
        for (int j = 0; j < board->COL_NUM; j++) {
            neighbors[i][j] = DEAD;
        }
    }

    // Inner cells
    for (int i = 1; i < board->ROW_NUM - 1; i++) { // Changed loop start and end to exclude first and last rows
        for (int j = 0; j < board->COL_NUM; j++) {
            i_prev = i - 1;
            i_next = i + 1;
            j_prev = (j > 0) ? j - 1 : board->COL_NUM - 1; // The previous cell in the row
            j_next = (j < (board->COL_NUM - 1)) ? j + 1 : 0; // The next cell in the row

            // calculate neighbors for the internal cells using only the board->cell_state array
            neighbors[i][j] = board->cell_state[i_prev][j_prev] +   // Top left
                              board->cell_state[i_prev][j] +        // Top
                              board->cell_state[i_prev][j_next] +   // Top right
                              board->cell_state[i][j_prev] +        // Left
                              board->cell_state[i][j_next] +        // Right
                              board->cell_state[i_next][j_prev] +   // Bottom left
                              board->cell_state[i_next][j] +        // Bottom
                              board->cell_state[i_next][j_next];    // Bottom right
        }
    }

    // If received ghost cells, calculate neighbors for them
    MPI_Waitall(*request_count, requests, statuses);

    // Outer cells (first and last rows)
    for (int j = 0; j < board->COL_NUM; j++) {
        j_prev = (j > 0) ? j - 1 : board->COL_NUM - 1; // The previous cell in the row
        j_next = (j < (board->COL_NUM - 1)) ? j + 1 : 0; // The next cell in the row

        // First row
        neighbors[0][j] = board->ghost_cell_state[0][j_prev] +      // Top left
                          board->ghost_cell_state[0][j] +           // Top
                          board->ghost_cell_state[0][j_next] +      // Top right
                          board->cell_state[0][j_prev] +            // Left
                          board->cell_state[0][j_next] +            // Right
                          board->cell_state[1][j_prev] +            // Bottom left
                          board->cell_state[1][j] +                 // Bottom
                          board->cell_state[1][j_next];             // Bottom right

        // Last row
        int last = board->ROW_NUM - 1;
        neighbors[last][j] = board->cell_state[last - 1][j_prev] +  // Top left
                             board->cell_state[last - 1][j] +       // Top
                             board->cell_state[last - 1][j_next] +  // Top right
                             board->cell_state[last][j_prev] +      // Left
                             board->cell_state[last][j_next] +      // Right
                             board->ghost_cell_state[1][j_prev] +   // Bottom left
                             board->ghost_cell_state[1][j] +        // Bottom
                             board->ghost_cell_state[1][j_next];    // Bottom right
    }
}



void count_neighbors_spherical_world(board_t *board, unsigned char **neighbors) {
    int i_prev, i_next, j_prev, j_next;

    // Clear neighbors
    for (int i = 0; i < board->ROW_NUM; i++) {
        for (int j = 0; j < board->COL_NUM; j++) {
            neighbors[i][j] = DEAD;
        }
    }

    // Inner cells
    for (int i = 0; i < (board->COL_NUM); i++) {
        for (int j = 0; j < (board->ROW_NUM); j++) {
            i_prev = (i > 0) ? i - 1 : (board->COL_NUM - 1);                    // The previous cell in the row
            i_next = (i < (board->COL_NUM - 1)) ? i + 1 : 0;                    // The next cell in the row
            j_prev = (j > 0) ? j - 1 : (board->ROW_NUM - 1);                    // The previous row
            j_next = (j < (board->ROW_NUM - 1)) ? j + 1 : (board->ROW_NUM - 1); // The next row

            //print every next and prev
            //printf("i_prev: %d, i_next: %d, j_prev: %d, j_next: %d\n", i_prev, i_next, j_prev, j_next);
            //Crashes just here
            if (board->cell_state[i_prev][j_prev] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i][j_prev] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i_next][j_prev] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i_prev][j] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i_next][j] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i_prev][j_next] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i][j_next] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i_next][j_next] == ALIVE) {
                neighbors[i][j]++;
            }
        }
    }
}

void count_neighbors_flat_world(board_t *board, unsigned char **neighbors) {
    // Clear neighbors
    for (int i = 0; i < board->COL_NUM; i++) {
        for (int j = 0; j < board->ROW_NUM; j++) {
            neighbors[i][j] = DEAD;
        }
    }

    // Inner cells
    for (int i = 1; i < (board->COL_NUM - 1); i++) {
        for (int j = 1; j < (board->ROW_NUM - 1); j++) {
            if (board->cell_state[i - 1][j - 1] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i][j - 1] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i + 1][j - 1] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i - 1][j] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i + 1][j] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i - 1][j + 1] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i][j + 1] == ALIVE) {
                neighbors[i][j]++;
            }
            if (board->cell_state[i + 1][j + 1] == ALIVE) {
                neighbors[i][j]++;
            }
        }
    }

    // Top cells
    for (int i = 1; i < (board->COL_NUM - 1); i++) {
        if (board->cell_state[i - 1][0] == ALIVE) {
            neighbors[i][0]++;
        }
        if (board->cell_state[i - 1][1] == ALIVE) {
            neighbors[i][0]++;
        }
        if (board->cell_state[i][1] == ALIVE) {
            neighbors[i][0]++;
        }
        if (board->cell_state[i + 1][1] == ALIVE) {
            neighbors[i][0]++;
        }
        if (board->cell_state[i + 1][0] == ALIVE) {
            neighbors[i][0]++;
        }
    }

    // Left cells
    for (int j = 1; j < (board->ROW_NUM - 1); j++) {
        if (board->cell_state[0][j - 1] == ALIVE) {
            neighbors[0][j]++;
        }
        if (board->cell_state[1][j - 1] == ALIVE) {
            neighbors[0][j]++;
        }
        if (board->cell_state[1][j] == ALIVE) {
            neighbors[0][j]++;
        }
        if (board->cell_state[1][j + 1] == ALIVE) {
            neighbors[0][j]++;
        }
        if (board->cell_state[0][j + 1] == ALIVE) {
            neighbors[0][j]++;
        }
    }

    // Bottom cells
    for (int i = 1; i < (board->COL_NUM - 1); i++) {
        if (board->cell_state[i - 1][board->ROW_NUM - 1] == ALIVE) {
            neighbors[i][board->ROW_NUM - 1]++;
        }
        if (board->cell_state[i - 1][board->ROW_NUM - 2] == ALIVE) {
            neighbors[i][board->ROW_NUM - 1]++;
        }
        if (board->cell_state[i][board->ROW_NUM - 2] == ALIVE) {
            neighbors[i][board->ROW_NUM - 1]++;
        }
        if (board->cell_state[i + 1][board->ROW_NUM - 2] == ALIVE) {
            neighbors[i][board->ROW_NUM - 1]++;
        }
        if (board->cell_state[i + 1][board->ROW_NUM - 1] == ALIVE) {
            neighbors[i][board->ROW_NUM - 1]++;
        }

    }
    // Right cells
    for (int j = 1; j < (board->ROW_NUM - 1); j++) {
        if (board->cell_state[board->COL_NUM - 1][j - 1] == ALIVE) {
            neighbors[board->COL_NUM - 1][j]++;
        }
        if (board->cell_state[board->COL_NUM - 2][j - 1] == ALIVE) {
            neighbors[board->COL_NUM - 1][j]++;
        }
        if (board->cell_state[board->COL_NUM - 2][j] == ALIVE) {
            neighbors[board->COL_NUM - 1][j]++;
        }
        if (board->cell_state[board->COL_NUM - 2][j + 1] == ALIVE) {
            neighbors[board->COL_NUM - 1][j]++;
        }
        if (board->cell_state[board->COL_NUM - 1][j + 1] == ALIVE) {
            neighbors[board->COL_NUM - 1][j]++;
        }
    }

    // Top left corner
    if (board->cell_state[1][0] == ALIVE)
        neighbors[0][0]++;
    if (board->cell_state[1][1] == ALIVE)
        neighbors[0][0]++;
    if (board->cell_state[0][1] == ALIVE)
        neighbors[0][0]++;

    // Bottom left corner
    if (board->cell_state[1][board->ROW_NUM - 1] == ALIVE)
        neighbors[0][board->ROW_NUM - 1]++;
    if (board->cell_state[1][board->ROW_NUM - 2] == ALIVE)
        neighbors[0][board->ROW_NUM - 1]++;
    if (board->cell_state[0][board->ROW_NUM - 2] == ALIVE)
        neighbors[0][board->ROW_NUM - 1]++;

    // Bottom right corner
    if (board->cell_state[board->COL_NUM - 2][board->ROW_NUM - 1] == ALIVE)
        neighbors[board->COL_NUM - 1][board->ROW_NUM - 1]++;
    if (board->cell_state[board->COL_NUM - 1][board->ROW_NUM - 2] == ALIVE)
        neighbors[board->COL_NUM - 1][board->ROW_NUM - 1]++;
    if (board->cell_state[board->COL_NUM - 2][board->ROW_NUM - 2] == ALIVE)
        neighbors[board->COL_NUM - 1][board->ROW_NUM - 1]++;

    // Top left corner
    if (board->cell_state[board->COL_NUM - 1][1] == ALIVE)
        neighbors[board->COL_NUM - 1][0]++;
    if (board->cell_state[board->COL_NUM - 2][1] == ALIVE)
        neighbors[board->COL_NUM - 1][0]++;
    if (board->cell_state[board->COL_NUM - 2][0] == ALIVE)
        neighbors[board->COL_NUM - 1][0]++;
}

void evolve(board_t *board, unsigned char **neighbors) {
    for (int i = 0; i < board->ROW_NUM; i++) {
        for (int j = 0; j < board->COL_NUM; j++) {
            // underopulation case
            if (neighbors[i][j] < 2) {
                board->cell_state[i][j] = DEAD;
            }
                // birth case
            else if (neighbors[i][j] == 3)
                board->cell_state[i][j] = ALIVE;
                // overpopulation case
            else if (neighbors[i][j] > 3)
                board->cell_state[i][j] = DEAD;
            // survival case is implicit, as only cells with 2 or 3 neighbors will
            // survive.
        }
    }
}


/******************************************************************************/

void life_read(char *filename, board_t *board)

/******************************************************************************/
/*
  Purpose:
    LIFE_READ reads a file to a grid.

  Parameters:

    Input, char *OUTPUT_FILENAME, the input file name.

*/

{
    FILE *input_unit;
/*
  input the file.
*/
    input_unit = fopen(filename, "rt");
    if (input_unit == NULL)
        perror("Reading input file:");
/*
  Read the data.
*/
    for (int i = 0; i < board->COL_NUM; i++) {
        for (int j = 0; j < board->ROW_NUM; j++) {
            fscanf(input_unit, "%hhu", &(board->cell_state[i][j]));
        }
    }
/*
  Close the file.
*/
    fclose(input_unit);

    return;
}


/******************************************************************************/

void life_write(char *output_filename, board_t *board)

/******************************************************************************/
/*
  Purpose:

    LIFE_WRITE writes a boad to a file.

  Parameters:

    Input, char *OUTPUT_FILENAME, the output file name.

*/
{
    FILE *output_unit;
/*
  Open the file.
*/
    output_unit = fopen(output_filename, "wt");
/*
  Write the data.
*/
    for (int i = 0; i < board->COL_NUM; i++) {
        for (int j = 0; j < board->ROW_NUM; j++) {
            fprintf(output_unit, " %hhu", board->cell_state[i][j]);
        }
        fprintf(output_unit, "\n");
    }
/*
  Close the file.
*/
    fclose(output_unit);

    return;
}

/******************************************************************************/

double r8_uniform_01(int *seed)

/******************************************************************************/
/*
  Purpose:

    R8_UNIFORM_01 returns a pseudorandom R8 scaled to [0,1].

  Discussion:

    This routine implements the recursion

      seed = 16807 * seed mod ( 2^31 - 1 )
      r8_uniform_01 = seed / ( 2^31 - 1 )

    The integer arithmetic never requires more than 32 bits,
    including a sign bit.

    If the initial seed is 12345, then the first three computations are

      Input     Output      R8_UNIFORM_01
      SEED      SEED

         12345   207482415  0.096616
     207482415  1790989824  0.833995
    1790989824  2035175616  0.947702

  Parameters:

    Input/output, int *SEED, the "seed" value.  Normally, this
    value should not be 0.  On output, SEED has been updated.

    Output, double R8_UNIFORM_01, a new pseudorandom variate, strictly between
    0 and 1.
*/
{
    int i4_huge = 2147483647;
    int k;
    double r;

    k = *seed / 127773;

    *seed = 16807 * (*seed - k * 127773) - k * 2836;

    if (*seed < 0) {
        *seed = *seed + i4_huge;
    }

    r = ((double) (*seed)) * 4.656612875E-10;

    return r;
}

/******************************************************************************/

void life_init(board_t *board, double prob, int *seed)

/******************************************************************************/
/*
  Purpose:

    LIFE_INIT initializes the life grid.

  Parameters:

    Input, double PROB, the probability that a grid cell
    should be alive.

    Input/output, int *SEED, a seed for the random
    number generator.

*/
{
    double r;

    for (int i = 0; i < board->COL_NUM; i++) {
        for (int j = 0; j < board->ROW_NUM; j++) {
            board->cell_state[i][j] = 0;
        }
    }

    for (int i = 0; i < board->COL_NUM; i++) {
        for (int j = 0; j < board->ROW_NUM; j++) {
            r = r8_uniform_01(seed);
            if (r <= prob) {
                board->cell_state[i][j] = 1;
            }
        }
    }
}



/*
// Top cells
for (int i = 1; i < (board->COL_NUM - 1); i++) {
  if (board->cell_state[i-1][0] == ALIVE) {
    neighbors[i][0]++;
  }
  if (board->cell_state[i-1][1] == ALIVE) {
    neighbors[i][0]++;
  }
  if (board->cell_state[i][1] == ALIVE) {
    neighbors[i][0]++;
  }
  if (board->cell_state[i+1][1] == ALIVE) {
    neighbors[i][0]++;
  }
  if (board->cell_state[i+1][0] == ALIVE) {
    neighbors[i][0]++;
  }
}

// Left cells
for (int j = 1; j < (board->ROW_NUM - 1); j++) {
  if (board->cell_state[0][j-1] == ALIVE) {
    neighbors[0][j]++;
  }
  if (board->cell_state[1][j-1] == ALIVE) {
    neighbors[0][j]++;
  }
  if (board->cell_state[1][j] == ALIVE) {
    neighbors[0][j]++;
  }
  if (board->cell_state[1][j+1] == ALIVE) {
    neighbors[0][j]++;
  }
  if (board->cell_state[0][j+1] == ALIVE) {
    neighbors[0][j]++;
  }
}

// Bottom cells
for (int i = 1; i < (board->COL_NUM - 1); i++) {
  if (board->cell_state[i-1][board->ROW_NUM - 1] == ALIVE) {
    neighbors[i][board->ROW_NUM - 1]++;
  }
  if (board->cell_state[i-1][board->ROW_NUM - 2] == ALIVE) {
    neighbors[i][board->ROW_NUM - 1]++;
  }
  if (board->cell_state[i][board->ROW_NUM - 2] == ALIVE) {
    neighbors[i][board->ROW_NUM - 1]++;
  }
  if (board->cell_state[i+1][board->ROW_NUM - 2] == ALIVE) {
    neighbors[i][board->ROW_NUM - 1]++;
  }
  if (board->cell_state[i+1][board->ROW_NUM - 1] == ALIVE) {
    neighbors[i][board->ROW_NUM - 1]++;
  }

}
// Right cells
for (int j = 1; j < (board->ROW_NUM - 1); j++) {
  if (board->cell_state[board->COL_NUM - 1][j-1] == ALIVE) {
    neighbors[board->COL_NUM - 1][j]++;
  }
  if (board->cell_state[board->COL_NUM - 2][j-1] == ALIVE) {
    neighbors[board->COL_NUM - 1][j]++;
  }
  if (board->cell_state[board->COL_NUM - 2][j] == ALIVE) {
    neighbors[board->COL_NUM - 1][j]++;
  }
  if (board->cell_state[board->COL_NUM - 2][j+1] == ALIVE) {
    neighbors[board->COL_NUM - 1][j]++;
  }
  if (board->cell_state[board->COL_NUM - 1][j+1] == ALIVE) {
    neighbors[board->COL_NUM - 1][j]++;
  }
}

// Top left corner
if (board->cell_state[1][0] == ALIVE)
  neighbors[0][0]++;
if (board->cell_state[1][1] == ALIVE)
  neighbors[0][0]++;
if (board->cell_state[0][1] == ALIVE)
  neighbors[0][0]++;

// Bottom left corner
if (board->cell_state[1][board->ROW_NUM - 1] == ALIVE)
  neighbors[0][board->ROW_NUM - 1]++;
if (board->cell_state[1][board->ROW_NUM - 2] == ALIVE)
  neighbors[0][board->ROW_NUM - 1]++;
if (board->cell_state[0][board->ROW_NUM - 2] == ALIVE)
  neighbors[0][board->ROW_NUM - 1]++;

// Bottom right corner
if (board->cell_state[board->COL_NUM - 2][board->ROW_NUM - 1] == ALIVE)
  neighbors[board->COL_NUM - 1][board->ROW_NUM - 1]++;
if (board->cell_state[board->COL_NUM - 1][board->ROW_NUM - 2] == ALIVE)
  neighbors[board->COL_NUM - 1][board->ROW_NUM - 1]++;
if (board->cell_state[board->COL_NUM - 2][board->ROW_NUM - 2] == ALIVE)
  neighbors[board->COL_NUM - 1][board->ROW_NUM - 1]++;

// Top left corner
if (board->cell_state[board->COL_NUM - 1][1] == ALIVE)
  neighbors[board->COL_NUM - 1][0]++;
if (board->cell_state[board->COL_NUM - 2][1] == ALIVE)
  neighbors[board->COL_NUM - 1][0]++;
if (board->cell_state[board->COL_NUM - 2][0] == ALIVE)
  neighbors[board->COL_NUM - 1][0]++;*/