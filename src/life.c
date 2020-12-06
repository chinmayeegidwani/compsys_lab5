/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <pthread.h>
#include <stdbool.h>

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
#define NUM_THREADS 16
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
	s = BOARD(inboard, i,   j); \
	sw = BOARD(inboard, i, jwest); \
	se = BOARD(inboard, i, jeast); \
	hash = (nw ? 256 : 0) + (n ? 128 : 0) + (ne ? 64 : 0) + (w ? 32 : 0)+ (curr ? 16 : 0) + (e ? 8 : 0) + (sw ? 4 : 0) + (s ? 2 : 0) +(se ? 1 : 0); \
	BOARD(outboard, i-1, j) = lookup[hash]

bool lookup[511]; //2^9 possibilities

void populate_lookup(){
	/* The idea here is to populate with every possibility of
	 * surrounding cells being alive or dead.
	 */
	int curr, nw, n, ne, w, e, sw, s, se;
	int alive_count, hash;
  for (int nw = 0; nw < 2; nw++) {
    for (int n = 0; n < 2; n++) {
       for (int ne = 0; ne <2; ne++) {
         for (int w = 0; w <2; w++) {
            for (int curr = 0; curr <2; curr++) {
              for (int e = 0; e <2; e++) {
                 for (int sw = 0; sw <2; sw++) {
                  for (int s = 0; s <2; s++) {
                    for (int se = 0; se <2; se++) {
                       hash = nw * 256 + n * 128 + ne * 64 + w * 32 + curr * 16
                      + e * 8 + sw * 4 + s * 2 + se * 1;
                       alive_count = nw + n + ne + w + e + sw + s + se;

						/* Check the rules and determine the next state given current environment
								* 1. Any live cell with two or three live neighbours survives.
								* 2. Any dead cell with three live neighbours becomes a live cell.
								* 3. All other live cells die in the next generation. Similarly, all other dead cells stay dead.
						*/
						if((curr == 1 && alive_count == 2) || (curr == 1 && alive_count == 3)) lookup[hash] = 1;
						else if(curr == 0 && alive_count == 3) lookup[hash] = 1;
						else lookup[hash] = 0;
                  }   
                }   
              }   
            }   
          }
        }
      }
    }
  }
}

char* single(char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max){
    int curgen, i, j;
	const int rows_per_thread = nrows/NUM_THREADS;
	char n, s, w, e, nw, ne, sw, se, curr, count;
	const int LDA = nrows;
	int jwest, jeast, hash;

	for(curgen=0; curgen < gens_max; curgen++){
    	for (j = 0; j < nrows; j++) {
				 int jwest = (j == 0)? (ncols-1):(j-1);
				 int jeast = (j == ncols-1) ? (0):(j+1);

				n = BOARD(inboard, nrows-2, j); // 128
				ne = BOARD(inboard, nrows-2, jeast);// 64
				nw = BOARD(inboard, nrows-2, jwest); // 256
				e = BOARD(inboard, nrows-1, jeast);// 8
				w = BOARD(inboard, nrows-1, jwest);// 32
				s = BOARD(inboard, 0, j); // 2
				se = BOARD(inboard, 0, jeast);// 1
				sw = BOARD(inboard, 0, jwest);// 4
				curr = BOARD(inboard, nrows-1, j); /// 16
				// nw*256 + n*128 + ne*64 + w*32 + curr*16 + e*8 + sw*4 + s*2 + se;
				hash = (nw ? 256 : 0) + (n ? 128 : 0) + (ne ? 64 : 0) + (w ? 32 : 0)
						+ (curr ? 16 : 0) + (e ? 8 : 0) + (sw ? 4 : 0) + (s ? 2 : 0) +(se ? 1 : 0);

				BOARD(outboard, nrows-1, j) = lookup[hash];

			for (i = 1; i < nrows; i++) {
					nw = w; 
					ne = e;  
					n = curr; 
					curr = s; 
					e = se; 
					w = sw; 
					s = BOARD(inboard, i,   j); 
					sw = BOARD(inboard, i, jwest); 
					se = BOARD(inboard, i, jeast); 

				hash = (nw ? 256 : 0) + (n ? 128 : 0) + (ne ? 64 : 0) + (w ? 32 : 0)
						+ (curr ? 16 : 0) + (e ? 8 : 0) + (sw ? 4 : 0) + (s ? 2 : 0) +(se ? 1 : 0);

				BOARD(outboard, i-1, j) = lookup[hash];
			}
    }

    SWAP_BOARDS( outboard, inboard );

  }

  return inboard;

}


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

	populate_lookup();
		//create_lookup_table();
	//return single(outboard, inboard, nrows, ncols, gens_max);

	pthread_t* threads = malloc(sizeof(pthread_t) * NUM_THREADS);
	thread_info* thread_args = malloc(sizeof(thread_info) * NUM_THREADS);
	pthread_barrier_t* barrier_per_gen = malloc(sizeof(pthread_barrier_t));
	pthread_barrier_init(barrier_per_gen, 0, NUM_THREADS);
	int rows_per_thread = nrows/NUM_THREADS;
	int row = 0;
	int i;
	for(i=0; i<NUM_THREADS; i++){
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
*/

void* process_thread(void* arg){
	thread_info* args = (thread_info*) arg;
	const int nrows = args->nrows;
	const int ncols = args->ncols;
	char* inboard = args->inboard;
	char* outboard = args->outboard;
	pthread_barrier_t* barrier = args->barrier_per_gen;
	int gens_max = args->gens_max;
	int start = args->start;
	int end = args->end; //denotes start and end columns


	
    int curgen, i, j;
	const int rows_per_thread = nrows/NUM_THREADS;
	char n, s, w, e, nw, ne, sw, se, curr, count;
	const int LDA = nrows;
	int jwest, jeast, hash;


	for(curgen=0; curgen < gens_max; curgen++){
    	for (j = 0; j < nrows; j++) {
				 int jwest = (j == 0)? (ncols-1):(j-1);
				 int jeast = (j == ncols-1) ? (0):(j+1);

				n = BOARD(inboard, nrows-2, j); // 128
				ne = BOARD(inboard, nrows-2, jeast);// 64
				nw = BOARD(inboard, nrows-2, jwest); // 256
				e = BOARD(inboard, nrows-1, jeast);// 8
				w = BOARD(inboard, nrows-1, jwest);// 32
				s = BOARD(inboard, 0, j); // 2
				se = BOARD(inboard, 0, jeast);// 1
				sw = BOARD(inboard, 0, jwest);// 4
				curr = BOARD(inboard, nrows-1, j); /// 16
				// nw*256 + n*128 + ne*64 + w*32 + curr*16 + e*8 + sw*4 + s*2 + se;
				hash = (nw ? 256 : 0) + (n ? 128 : 0) + (ne ? 64 : 0) + (w ? 32 : 0)
						+ (curr ? 16 : 0) + (e ? 8 : 0) + (sw ? 4 : 0) + (s ? 2 : 0) +(se ? 1 : 0);

				BOARD(outboard, nrows-1, j) = lookup[hash];

			for (i = 1; i < nrows; i++) {
					nw = w; 
					ne = e;  
					n = curr; 
					curr = s; 
					e = se; 
					w = sw; 
					s = BOARD(inboard, i,   j); 
					sw = BOARD(inboard, i, jwest); 
					se = BOARD(inboard, i, jeast); 

				hash = (nw ? 256 : 0) + (n ? 128 : 0) + (ne ? 64 : 0) + (w ? 32 : 0)
						+ (curr ? 16 : 0) + (e ? 8 : 0) + (sw ? 4 : 0) + (s ? 2 : 0) +(se ? 1 : 0);

				BOARD(outboard, i-1, j) = lookup[hash];
			}
    	}
		// do at the end of a generation
		SWAP_BOARDS(outboard, inboard);
		pthread_barrier_wait(barrier);
	}

}
