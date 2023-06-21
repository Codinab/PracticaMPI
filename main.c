#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <mpi.h>

#include "./game.h"
#include "./logic.h"
#include "./render.h"


void free_neighbors(int col_num, unsigned char **neighbors);

void init_neighbors(int col_num, int row_num, unsigned char **neighbors);


void parse_arguments(int argc, char **argv, char *input_file, char *output_file, bool *LoadFile, bool *SaveFile,
                     int *iterations, int *col_num, int *row_num);

void create_board(int col_num, int row_num, board_t *board);

void usage() {
    printf("\n -g\tEnable graphical mode.\n\n");
    printf("\n -w\tSet board weight.\n\n");
    printf("\n -h\tSet board height.\n\n");
    printf("\n -i\tInput board file.\n\n");
    printf("\n -o\tOutput board file.\n\n");
    printf("\n -e\tNumber of simulation iterations.\n\n");
    printf("Enter extremely low values at own peril.\n\tRecommended to stay in 30000-100000 range.\n\tDefaults to 50000.\n\n");
    printf("\n -c\tSet cell size to tiny, small, medium or large.\n\tDefaults to small.\n\n");
}

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Process %d of %d generated\n", rank, size);

    bool LoadFile = false, SaveFile = false;
    int iterations = -1;
    char input_file[256], output_file[256];

    int col_num = D_COL_NUM;
    int row_num = D_ROW_NUM;

    // Command line options.
    parse_arguments(argc, argv, input_file, output_file, &LoadFile, &SaveFile, &iterations, &col_num, &row_num);

    // Calculate the number of rows each process should handle.
    int rows_for_process = row_num / size;
    int remainder = row_num % size;

    // If the total number of rows isn't perfectly divisible by the number of processes,
    // distribute the extra rows to the first few processes.
    int local_row_num = rows_for_process;
    if (rank < remainder) {
        ++local_row_num;
    }

    board_t *board = (board_t *) malloc(sizeof(board_t));
    create_board(col_num, local_row_num, board);

    //printf("Rank %d: COL_NUM = %d, ROW_NUM = %d, Total ROW_NUM = %d\n", rank, board->COL_NUM, board->ROW_NUM, row_num);

    unsigned char **neighbors = malloc((local_row_num + 2) * sizeof(unsigned char *));
    init_neighbors(col_num, (local_row_num + 2), neighbors);

    board_t *board_full_size = (board_t *) malloc(sizeof(board_t));
    create_board(col_num, row_num, board_full_size);


    if (rank == 0) {

        if (LoadFile) {
            printf("Loading Board file %s.\n", input_file);
            life_read(input_file, board_full_size);
        } else { // Rando, init file
            printf("Init Cells\n");
            fflush(stdout);
            double prob = 0.20;
            int seed = 123456789;
            life_init(board_full_size, prob, &seed);
        }

        int destRank = 0;
        int sendcounts[size];

        for (int i = 0; i < size; i++) {
            sendcounts[i] = rows_for_process;
            if (i < remainder) {
                sendcounts[i]++;
            }
        }

        for (int i = 0; i < board_full_size->ROW_NUM; i++) {
            MPI_Send(board_full_size->cell_state[i], board->COL_NUM, MPI_UNSIGNED_CHAR, destRank, 0, MPI_COMM_WORLD);
            sendcounts[destRank]--;
            if(sendcounts[destRank] <= 0) {
                destRank++;
            }
        }
    }
    for (int i = 0; i < board->ROW_NUM; i++) {
        MPI_Recv(board->cell_state[i], board->COL_NUM, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    render_board(board, neighbors, rank == size-1 ? 0: rank+1);
    exit(0);
    printf("Start Simulation.\n");
    fflush(stdout);
    int Iteration = 0;
    while ((iterations < 0 || Iteration < iterations) != 0) {
        //render_board(board, neighbors, rank == size-1 ? 0: rank+1);
        printf("[%05d] Life Game Simulation step.\r", ++Iteration);
        fflush(stdout);
    }

    printf("\nEnd Simulation.\n");

    if (SaveFile) {
        printf("Writing Board file %s.\n", output_file);
        fflush(stdout);
        life_write(output_file, board);
    }

     //Recolectar los board->cell_state de cada proceso en el proceso principal (rank 0)
    //MPI_Gatherv(board->cell_state, sendcounts[rank] * board->COL_NUM, MPI_INT,
    //            board_full_size->cell_state, sendcounts, displs, MPI_INT,
    //            0, MPI_COMM_WORLD);
    //


    if (rank == 0) {


        // Ejemplo: Imprimir el board completo
        for (int i = 0; i < row_num; i++) {
            for (int j = 0; j < col_num; j++) {
                printf("%d ", board_full_size->cell_state[i][j]);
            }
            printf("\n");
        }
    }


    if (rank == 0) {
        free_board(board);
    }

    free_board(board);
    //free_neighbors(col_num, neighbors);

    MPI_Finalize();

    return EXIT_SUCCESS;
}

void create_board(int col_num, int row_num, board_t *board) {
    if (board == NULL) {
        fprintf(stderr, "Error reserving board memory %lf KB", sizeof(board_t) / 1024.0);
        exit(1);
    }
    // Configure board initial state.
    board->game_state = RUNNING_STATE;
    board->COL_NUM = col_num;
    board->ROW_NUM = row_num;
    allocate_board(board);
    init_board(board);
}

void parse_arguments(int argc, char **argv, char *input_file, char *output_file, bool *LoadFile, bool *SaveFile,
                     int *iterations, int *col_num, int *row_num) {
    int opt;
    while ((opt = getopt(argc, argv, ":h:i:o:w:H:e:")) != -1) {
        switch (opt) {
            case 'i':
                strcpy(input_file, optarg);
                (*LoadFile) = true;
                break;
            case 'o':
                (*SaveFile) = true;
                strcpy(output_file, optarg);
                printf("Output Board file %s.\n", optarg);
                break;
            case 'w':
                (*col_num) = atoi(optarg);
                printf("Board width %d.\n", (*col_num));
                break;
            case 'h':
                (*row_num) = atoi(optarg);
                printf("Board height %d.\n", (*row_num));
                break;
            case 'e':
                printf("End Time: %s.\n", optarg);
                (*iterations) = atoi(optarg);
                break;
            case 'H':
                usage();
                exit(EXIT_SUCCESS);
            case '?':
                if (optopt == 't' || optopt == 's' || optopt == 'c' || optopt == 'i' || optopt == 'o' ||
                    optopt == 'w' || optopt == 'h' || optopt == 'e') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else if (isprint (optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                }
                printf("Setting default options.\n");
                usage();
                break;
            default:
                printf("Setting default options.\n");
                usage();
                break;
        }
    }
}

void init_neighbors(int col_num, int row_num, unsigned char **neighbors) {
    for (int i = 0; i < row_num; i++) {
        neighbors[i] = malloc(col_num * sizeof(unsigned char));
        for (int j = 0; j < col_num; j++) {
            neighbors[i][j] = DEAD;
        }
    }
}

void free_neighbors(int col_num, unsigned char **neighbors) {
    for (int i = 0; i < col_num; i++) {
        free(neighbors[i]);
    }
    free(neighbors);
}

void init_board(board_t *board) {
    printf("Initializing COL_NUM %d, ROW_NUM %d\n", board->COL_NUM, board->ROW_NUM);
    for (int i = 0; i < board->ROW_NUM; i++) {
        for (int j = 0; j < board->COL_NUM; j++) {
            board->cell_state[i][j] = 0;
            //printf("%d ", board->cell_state[i][j]);
        }
        //printf("\n");
    }
}

void free_board(board_t *board) {
    for (int i = 0; i < board->COL_NUM; i++) {
        free(board->cell_state[i]);
    }
    free(board->cell_state);
    free(board);
}

void allocate_board(board_t *board) {
    printf("Allocating COL_NUM %d, ROW_NUM %d\n", board->COL_NUM, board->ROW_NUM);
    board->cell_state = (unsigned char **) malloc(sizeof(unsigned char *) * board->ROW_NUM);
    if (board->cell_state == NULL) {
        fprintf(stderr, "Error reserving board memory %lf KB",
                sizeof(unsigned char *) * board->COL_NUM + sizeof(unsigned char) * board->ROW_NUM * board->ROW_NUM / 1024.0);
        exit(1);
    }
    for (int i = 0; i < board->ROW_NUM; i++) {
        board->cell_state[i] = (unsigned char *) malloc(sizeof(unsigned char) * board->COL_NUM);
        if (board->cell_state[i] == NULL) {
            fprintf(stderr, "Error reserving board memory %lf KB", sizeof(unsigned char) * board->COL_NUM / 1024.0);
            exit(1);
        }
    }
}

