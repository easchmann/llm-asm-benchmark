/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* atax.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
#include "atax.h"


/* Array initialization. */
static
void init_array (int m, int n,
		 DATA_TYPE POLYBENCH_2D(A,M,N,m,n),
		 DATA_TYPE POLYBENCH_1D(x,N,n))
{
  int i, j;
  DATA_TYPE fn;
  fn = (DATA_TYPE)n;

  for (i = 0; i < n; i++)
      x[i] = 1 + (i / fn);
  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
      A[i][j] = (DATA_TYPE) ((i+j) % n) / (5*m);
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int n,
		 DATA_TYPE POLYBENCH_1D(y,N,n))

{
  int i;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("y");
  for (i = 0; i < n; i++) {
    if (i % 20 == 0) fprintf (POLYBENCH_DUMP_TARGET, "\n");
    fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, y[i]);
  }
  POLYBENCH_DUMP_END("y");
  POLYBENCH_DUMP_FINISH;
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_atax(int m, int n,
		 DATA_TYPE POLYBENCH_2D(A,M,N,m,n),
		 DATA_TYPE POLYBENCH_1D(x,N,n),
		 DATA_TYPE POLYBENCH_1D(y,N,n),
		 DATA_TYPE POLYBENCH_1D(tmp,M,m))
{
  int i, j;

#pragma scop
__asm__ volatile (
    "mov %1, %%ax\n\t"          /* ax = n */
    "mov %0, %%cx\n\t"          /* cx = m */
    "mov %2, %%bx\n\t"          /* bx = A */
    "mov %3, %%dx\n\t"          /* dx = x */
    "mov %4, %%di\n\t"          /* di = y */
    "mov %5, %%si\n\t"          /* si = tmp */
    "shl %%ax\n\t"
    "shl %%ax\n\t"
    "shl %%ax\n\t"              /* ax = 8*n */
    "push %%bp\n\t"
    "mov %%si, %%bp\n\t"        /* bp = tmp base */
    "mov %%di, %%si\n\t"
    "add %%ax, %%si\n\t"        /* si = end of y */
"1:\n\t"
    "fldz\n\t"
    "fstpl (%%di)\n\t"
    "add $8, %%di\n\t"
    "cmp %%si, %%di\n\t"
    "jb 1b\n\t"
    "mov %%si, %%di\n\t"
    "sub %%ax, %%di\n\t"        /* di = y again */
    "jcxz 9f\n\t"
"2:\n\t"
    "push %%cx\n\t"
    "fldz\n\t"
    "fstpl (%%bp)\n\t"          /* tmp[i] = 0.0 */
    "xor %%si, %%si\n\t"
"3:\n\t"
    "fldl (%%bx,%%si)\n\t"
    "fmull (%%dx,%%si)\n\t"
    "faddl (%%bp)\n\t"
    "fstpl (%%bp)\n\t"          /* tmp[i] += A[i][j]*x[j] */
    "add $8, %%si\n\t"
    "cmp %%ax, %%si\n\t"
    "jb 3b\n\t"
"4:\n\t"
    "fldl (%%bx,%%si)\n\t"
    "fmull (%%bp)\n\t"
    "faddl (%%di,%%si)\n\t"
    "fstpl (%%di,%%si)\n\t"     /* y[j] += A[i][j]*tmp[i] */
    "add $8, %%si\n\t"
    "cmp %%ax, %%si\n\t"
    "jb 4b\n\t"
    "add %%ax, %%bx\n\t"
    "add $8, %%bp\n\t"
    "pop %%cx\n\t"
    "loop 2b\n\t"
"9:\n\t"
    "pop %%bp"
    : : "m" (m), "m" (n), "m" (A), "m" (x), "m" (y), "m" (tmp)
    : "ax", "bx", "cx", "dx", "si", "cc", "memory");
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int m = M;
  int n = N;

  /* Variable declaration/allocation. */
  POLYBENCH_2D_ARRAY_DECL(A, DATA_TYPE, M, N, m, n);
  POLYBENCH_1D_ARRAY_DECL(x, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(y, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(tmp, DATA_TYPE, M, m);

  /* Initialize array(s). */
  init_array (m, n, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(x));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_atax (m, n,
	       POLYBENCH_ARRAY(A),
	       POLYBENCH_ARRAY(x),
	       POLYBENCH_ARRAY(y),
	       POLYBENCH_ARRAY(tmp));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(n, POLYBENCH_ARRAY(y)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(x);
  POLYBENCH_FREE_ARRAY(y);
  POLYBENCH_FREE_ARRAY(tmp);

  return 0;
}
