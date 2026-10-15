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
    "mov %1, %%cx\n\t"
    "xor %%ax, %%ax\n\t"
    "0:\n\t"
    "mov %%ax, (%4, %%cx, 2)\n\t"
    "dec %%cx\n\t"
    "jnz 0b\n\t"

    "xor %%bx, %%bx\n\t"
    "1:\n\t"
    "mov %%bx, %%ax\n\t"
    "mov %1, %%dx\n\t"
    "imul %%dx, %%ax\n\t"
    "shl $1, %%ax\n\t"
    "mov %2, %%si\n\t"
    "mov %3, %%di\n\t"
    "mov %4, %%bp\n\t"
    "mov %5, %%cx\n\t"
    "add %%ax, %%si\n\t"
    "add %%ax, %%di\n\t"
    "add %%ax, %%bp\n\t"
    "add %%ax, %%cx\n\t"

    "xor %%ax, %%ax\n\t"
    "mov %1, %%dx\n\t"
    "2:\n\t"
    "mov (%%si), %%bp\n\t"
    "imul (%%di), %%bp\n\t"
    "add %%bp, %%ax\n\t"
    "add $2, %%si\n\t"
    "add $2, %%di\n\t"
    "dec %%dx\n\t"
    "jnz 2b\n\t"
    "mov %%ax, (%%cx)\n\t"

    "mov %2, %%si\n\t"
    "mov %4, %%di\n\t"
    "add %%ax, %%si\n\t"
    "add %%ax, %%di\n\t"
    "mov %1, %%cx\n\t"
    "3:\n\t"
    "mov (%%si), %%bx\n\t"
    "imul %%ax, %%bx\n\t"
    "mov (%%di), %%bp\n\t"
    "add %%bx, %%bp\n\t"
    "mov %%bp, (%%di)\n\t"
    "add $2, %%si\n\t"
    "add $2, %%di\n\t"
    "dec %%cx\n\t"
    "jnz 3b\n\t"

    "inc %%bx\n\t"
    "cmp %%0, %%bx\n\t"
    "jb 1b\n\t"
    :
    : "m"(m), "m"(n), "m"(A), "m"(x), "m"(y), "m"(tmp)
    : "ax", "bx", "cx", "dx", "si", "di", "bp", "memory", "cc"
);
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
