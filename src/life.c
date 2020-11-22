/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"

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
    const int LDA = nrows;
    int curgen, i, j;
	char n, s, w, e, nw, ne, sw, se, curr;

    for (curgen = 0; curgen < gens_max; curgen++)
    {
        /* HINT: you'll be parallelizing these loop(s) by doing a
           geometric decomposition of the output */
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
		// the output of this gen becomes the input of the next gen
        SWAP_BOARDS( outboard, inboard );

    }
    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */
    return inboard;
}
