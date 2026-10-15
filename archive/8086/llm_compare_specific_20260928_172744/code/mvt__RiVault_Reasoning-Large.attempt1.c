/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* mvt.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
#include "mvt.h"


/* Array initialization. */
static
void init_array(int n,
		DATA_TYPE POLYBENCH_1D(x1,N,n),
		DATA_TYPE POLYBENCH_1D(x2,N,n),
		DATA_TYPE POLYBENCH_1D(y_1,N,n),
		DATA_TYPE POLYBENCH_1D(y_2,N,n),
		DATA_TYPE POLYBENCH_2D(A,N,N,n,n))
{
  int i, j;

  for (i = 0; i < n; i++)
    {
      x1[i] = (DATA_TYPE) (i % n) / n;
      x2[i] = (DATA_TYPE) ((i + 1) % n) / n;
      y_1[i] = (DATA_TYPE) ((i + 3) % n) / n;
      y_2[i] = (DATA_TYPE) ((i + 4) % n) / n;
      for (j = 0; j < n; j++)
	A[i][j] = (DATA_TYPE) (i*j % n) / n;
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int n,
		 DATA_TYPE POLYBENCH_1D(x1,N,n),
		 DATA_TYPE POLYBENCH_1D(x2,N,n))

{
  int i;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("x1");
  for (i = 0; i < n; i++) {
    if (i % 20 == 0) fprintf (POLYBENCH_DUMP_TARGET, "\n");
    fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, x1[i]);
  }
  POLYBENCH_DUMP_END("x1");

  POLYBENCH_DUMP_BEGIN("x2");
  for (i = 0; i < n; i++) {
    if (i % 20 == 0) fprintf (POLYBENCH_DUMP_TARGET, "\n");
    fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, x2[i]);
  }
  POLYBENCH_DUMP_END("x2");
  POLYBENCH_DUMP_FINISH;
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_mvt(int n,
		DATA_TYPE POLYBENCH_1D(x1,N,n),
		DATA_TYPE POLYBENCH_1D(x2,N,n),
		DATA_TYPE POLYBENCH_1D(y_1,N,n),
		DATA_TYPE POLYBENCH_1D(y_2,N,n),
		DATA_TYPE POLYBENCH_2D(A,N,N,n,n))
{
  int i, j;

#pragma scop
__asm__ __volatile__ (
    "movw %5, %%dx\n\t"                  /* dx = _PB_N */
    "orw %%dx, %%dx\n\t"
    "jz 4f\n\t"                          /* nothing to do when N == 0 */

    /* ---- first nest: for i, for j: x1[i] += A[i][j] * y_1[j] ---- */
    "pushw %%bp\n\t"                     /* save the frame pointer */
    "pushw %%dx\n\t"                     /* outer loop counter on the stack */
    "movw %0, %%di\n\t"                  /* di = &x1[0] */
    "movw %4, %%bp\n\t"                  /* bp = A (row 0) */
    "movw %2, %%bx\n\t"                  /* bx = &y_1[0] */
    "movw %%dx, %%cx\n\t"
    "addw %%cx, %%cx\n\t"                /* cx = 2*N = row size in bytes */
    "addw %%cx, %%bx\n\t"                /* bx = &y_1[N]  (end anchor) */
    "addw %%cx, %%bp\n\t"                /* bp = &A[0][N] (end anchor) */
    "movw %%cx, %%si\n\t"
    "negw %%si\n\t"                      /* si = -2*N */
    "1:\n\t"
    "movw (%%bp,%%si), %%ax\n\t"         /* ax = A[i][j] */
    "imulw (%%bx,%%si)\n\t"              /* dx:ax = ax * y_1[j] */
    "addw %%ax, (%%di)\n\t"              /* x1[i] += low(product) */
    "addw $2, %%si\n\t"
    "jnz 1b\n\t"
    "addw $2, %%di\n\t"                  /* di = &x1[i+1] */
    "addw %%cx, %%bp\n\t"                /* bp = anchor of row i+1 */
    "movw %%cx, %%si\n\t"
    "negw %%si\n\t"                      /* restart inner index */
    "popw %%ax\n\t"
    "decw %%ax\n\t"
    "pushw %%ax\n\t"
    "jnz 1b\n\t"
    "addw $2, %%sp\n\t"                  /* discard exhausted counter */
    "popw %%bp\n\t"                      /* restore the frame pointer */

    /* ---- second nest: for i, for j: x2[i] += A[j][i] * y_2[j] ----
       looped with j outer / i inner; within one row of A the walk
       is contiguous, y_2[j] is a constant for the inner loop */
    "pushw %%bp\n\t"                     /* save the frame pointer */
    "movw %4, %%bp\n\t"                  /* bp = A (row 0) */
    "addw %%cx, %%bp\n\t"                /* bp = &A[0][N] (row 0 anchor) */
    "movw %1, %%bx\n\t"                  /* bx = &x2[0] */
    "addw %%cx, %%bx\n\t"                /* bx = &x2[N] (end anchor) */
    "movw %3, %%di\n\t"                  /* di = &y_2[0] */
    "movw %5, %%ax\n\t"
    "pushw %%ax\n\t"                     /* outer loop counter slot */
    "movw %%cx, %%dx\n\t"
    "pushw %%dx\n\t"                     /* stride = 2*N slot (on top) */
    "2:\n\t"
    "popw %%dx\n\t"                      /* dx = stride (2*N) */
    "movw %%dx, %%si\n\t"
    "negw %%si\n\t"                      /* si = -2*N */
    "movw (%%di), %%cx\n\t"              /* cx = y_2[j] */
    "3:\n\t"
    "movw (%%bp,%%si), %%ax\n\t"         /* ax = A[j][i] */
    "imulw %%cx\n\t"                     /* dx:ax = ax * y_2[j] */
    "addw %%ax, (%%bx,%%si)\n\t"         /* x2[i] += low(product) */
    "addw $2, %%si\n\t"
    "jnz 3b\n\t"
    "addw %%dx, %%bp\n\t"                /* bp = anchor of row j+1 */
    "addw $2, %%di\n\t"                  /* di = &y_2[j+1] */
    "popw %%ax\n\t"
    "decw %%ax\n\t"
    "pushw %%ax\n\t"
    "pushw %%dx\n\t"                     /* push stride back on top */
    "jnz 2b\n\t"
    "addw $4, %%sp\n\t"                  /* discard counter and stride */
    "popw %%bp\n\t"                      /* restore the frame pointer */
    "4:\n\t"
    :
    : "m" (x1), "m" (x2), "m" (y_1), "m" (y_2), "m" (A),
      "g" ((int) _PB_N)
    : "ax", "bx", "cx", "dx", "si", "di", "cc", "memory");
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int n = N;

  /* Variable declaration/allocation. */
  POLYBENCH_2D_ARRAY_DECL(A, DATA_TYPE, N, N, n, n);
  POLYBENCH_1D_ARRAY_DECL(x1, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(x2, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(y_1, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(y_2, DATA_TYPE, N, n);


  /* Initialize array(s). */
  init_array (n,
	      POLYBENCH_ARRAY(x1),
	      POLYBENCH_ARRAY(x2),
	      POLYBENCH_ARRAY(y_1),
	      POLYBENCH_ARRAY(y_2),
	      POLYBENCH_ARRAY(A));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_mvt (n,
	      POLYBENCH_ARRAY(x1),
	      POLYBENCH_ARRAY(x2),
	      POLYBENCH_ARRAY(y_1),
	      POLYBENCH_ARRAY(y_2),
	      POLYBENCH_ARRAY(A));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(n, POLYBENCH_ARRAY(x1), POLYBENCH_ARRAY(x2)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(x1);
  POLYBENCH_FREE_ARRAY(x2);
  POLYBENCH_FREE_ARRAY(y_1);
  POLYBENCH_FREE_ARRAY(y_2);

  return 0;
}
