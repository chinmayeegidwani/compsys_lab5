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
#define BOARD( __board, __i, __j)  (__board[(__i) + LDA*(__j)])
#define NUM_THREADS 8
void* process_thread(void* _args);

typedef struct thread_info{
	//int gens_max;
	int nrows;
	int ncols;
	char* inboard;
	char* outboard;
	//int section_num;
	pthread_barrier_t* barrier_per_gen;
	int gens_max;
	//int LDA;
	int start;
	int end;
} thread_info;

#define UNROLL(i) \
	nw = w; \
	ne = e;  \
	n = curr; \
	curr = s; \
	e = se; \
	w = sw; \
	s = BOARD(inboard, i, j); \
	sw = BOARD(inboard, i, jwest); \
	se = BOARD(inboard, i, jeast); \
	BOARD(outboard, i-1, j) = alivep(n+ne+nw+e+w+s+se+sw, curr);



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
	char n, s, w, e, nw, ne, sw, se, curr;
	const int LDA = nrows;
	int jwest, jeast;
	const int nrows_1 = nrows - 1;
  	const int nrows_2 = nrows - 2;
	for(curgen=0; curgen < gens_max; curgen++){
		for (j = 0; j < ncols; j++){
				// LICM
				 jwest = (j == 0)? (ncols-1):(j-1);
				 jeast = (j == ncols-1) ? (0):(j+1);


				// try to reuse previous cell's computed values


				
				n = BOARD(inboard, nrows-2, j); //
				ne = BOARD(inboard, nrows-2, jeast);//
				nw = BOARD(inboard, nrows-2, jwest); //
				e = BOARD(inboard, nrows-1, jeast);//
				w = BOARD(inboard, nrows-1, jwest);//
				s = BOARD(inboard, 0, j); //
				se = BOARD(inboard, 0, jeast);//
				sw = BOARD(inboard, 0, jwest);//
				curr = BOARD(inboard, nrows-1, j); //

				BOARD(outboard, nrows-1, j) = alivep(n+ne+nw+e+w+s+se+sw, curr);



				for (i = 1; i < nrows-16; i+=16)	{
				//const int inorth = mod (i-1, nrows); // calculating neighbor positions
				//const int isouth = mod (i+1, nrows);
					UNROLL(i);
					UNROLL(i+1);
					UNROLL(i+2);
					UNROLL(i+3);
					UNROLL(i+4);
					UNROLL(i+5);
					UNROLL(i+6);
					UNROLL(i+7);

					UNROLL(i+8);
					UNROLL(i+9);
					UNROLL(i+10);
					UNROLL(i+11);
					UNROLL(i+12);
					UNROLL(i+13);
					UNROLL(i+14);
					UNROLL(i+15);
				}
					UNROLL(i);
					UNROLL(i+1);
					UNROLL(i+2);
					UNROLL(i+3);
					UNROLL(i+4);
					UNROLL(i+5);
					UNROLL(i+6);

					UNROLL(i+7);
					UNROLL(i+8);
					UNROLL(i+9);
					UNROLL(i+10);
					UNROLL(i+11);
					UNROLL(i+12);
					UNROLL(i+13);
					UNROLL(i+14);
			}
			// do at the end of a generation
		SWAP_BOARDS(outboard, inboard);
	}

    return inboard;











/*
	pthread_t* threads = malloc(sizeof(pthread_t) * NUM_THREADS);
	thread_info* thread_args = malloc(sizeof(thread_info) * NUM_THREADS);
	pthread_barrier_t* barrier_per_gen = malloc(sizeof(pthread_barrier_t));
	pthread_barrier_init(barrier_per_gen, 0, NUM_THREADS);
	int row = 0; 
	for(int i=0; i<NUM_THREADS; i++){
		thread_args[i].ncols = ncols;
		thread_args[i].nrows = nrows;
		thread_args[i].inboard = inboard;
		thread_args[i].outboard = outboard;
		//thread_args[i].section_num = i;
		thread_args[i].barrier_per_gen = barrier_per_gen;
		thread_args[i].gens_max = gens_max;
		thread_args[i].start = row;
		row = rows_per_thread * (i+1);
		thread_args[i].end = row;
	} 


	for(i=0; i<NUM_THREADS; i++){
		thread_args[i].inboard = inboard;
		thread_args[i].outboard = outboard;
		pthread_create(&threads[i], NULL, process_thread, (void*) &thread_args[i]);
	}

	for(i=0; i<NUM_THREADS; i++){
		pthread_join(threads[i], NULL);
	}


	free(threads);
	free(thread_args);
	free(barrier_per_gen);
*/

    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */

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

