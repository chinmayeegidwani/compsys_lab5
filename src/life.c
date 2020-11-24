/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <pthread.h>

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/
/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

// accesses the element at (i, j) at input board
#define BOARD( __board, __j, __i)  (__board[(__i) + LDA*(__j)])
#define NUM_THREADS 8
void* process_thread(void* _args);

typedef struct thread_info{
	//int gens_max;
	int nrows;
	int ncols;
	char* inboard;
	char* outboard;
	int section_num;
	pthread_barrier_t* barrier;
} thread_info;
/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */

    int curgen, i, j;
	const int rows_per_thread = nrows/NUM_THREADS;

	pthread_t* threads = malloc(sizeof(pthread_t) * NUM_THREADS);
	thread_info* thread_args = malloc(sizeof(thread_info) * NUM_THREADS);
	//pthread_barrier_t* barrier_per_gen = malloc(sizeof(pthread_barrier_t));
	//pthread_barrier_init(barrier_per_gen, 0, NUM_THREADS);
	for(int i=0; i<NUM_THREADS; i++){
		thread_args[i].ncols = ncols;
		thread_args[i].nrows = nrows;
		thread_args[i].inboard = inboard;
		thread_args[i].outboard = outboard;
		thread_args[i].section_num = i;
		//thread_args[i].barrier = barrier_per_gen;
	}

	for(curgen=0; curgen < gens_max; curgen++){
		for(i=0; i<NUM_THREADS; i++){
			thread_args[i].inboard = inboard;
			thread_args[i].outboard = outboard;
			pthread_create(&threads[i], NULL, process_thread, (void*) &thread_args[i]);
		}

		for(i=0; i<NUM_THREADS; i++){
			pthread_join(threads[i], NULL);
		}
		SWAP_BOARDS(outboard, inboard);
	}
	free(threads);
	free(thread_args);
	//free(barrier_per_gen);


    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */
    return inboard;
}

/*

typedef struct thread_info{
	int nrows;
	int ncols;
	char* inboard;
	char* outboard;
	int section_num;
} thread_info;
*/


void* process_thread(void* _args){
	thread_info* args = (thread_info*) _args;
	int nrows = args->nrows;
	int ncols = args->ncols;
	char* inboard = args->inboard;
	char* outboard = args->outboard; 
	int section_num = args->section_num;
	int i, j;
	char n, s, w, e, nw, ne, sw, se, curr;
	const int LDA = nrows;

	// cut the entire board into sections
	int section_h = nrows/NUM_THREADS;
	int section_beg = section_h * section_num;
	int section_end = section_h * (section_num + 1); // end of one section is beginning of next;

	for (i = 0; i < nrows; i++)
			{
				// LICM
				const int inorth = mod (i-1, nrows); // calculating neighbor positions
				const int isouth = mod (i+1, nrows);

				// try to reuse previous cell's computed values
				n = BOARD(inboard, inorth, mod(-1, ncols));
				ne = BOARD(inboard, inorth, 0); //ne begins at first col of each row
				// nw calculated later
				e = BOARD(inboard, i, 0);
				s = BOARD(inboard, isouth, mod(-1, ncols));
				se = BOARD(inboard, isouth, 0);
				//sw = calculated later
				curr = BOARD(inboard, i, mod(-1, ncols));

				for (j = 0; j < ncols; j++)
				{
					//const int jwest = mod (j-1, ncols);
					const int jeast = mod (j+1, ncols);
					nw = n;
					n = ne;
					sw = s;
					s = se;

					w = curr;
					curr = e;

					ne = BOARD(inboard, inorth, jeast);
					e = BOARD(inboard, i, jeast);
					se = BOARD(inboard, isouth, jeast);

					// add em up
					const char neighbor_count = nw + n + ne + e + se + s + sw + w;
					// determine if the current cell will be alive in next gen by checking its neighbors
					BOARD(outboard, i, j) = alivep (neighbor_count, curr);

				}
			}
}