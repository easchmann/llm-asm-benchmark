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
asm volatile (
    "mov %0, %%ax\n\t"
    "mov %1, %%bx\n\t"
    "mov %2, %%cx\n\t"
    "mov %3, %%dx\n\t"
    "mov %4, %%si\n\t"
    "mov %5, %%di\n\t"
    "xor %%bp, %%bp\n\t"
    "mov %%bx, %%ax\n\t"
    "mov %4, %%si\n\t"
    "y_zero:\n\t"
    "xor %%bp, %%bp\n\t"
    "mov %%bp, (%%si)\n\t"
    "add $2, %%si\n\t"
    "dec %%ax\n\t"
    "jnz y_zero\n\t"
    "outer:\n\t"
    "mov %0, %%bp\n\t"
    "xor %%ax, %%ax\n\t"
    "mov %%ax, (%%di)\n\t"
    "mov %%ax, %%bx\n\t"
    "mov %1, %%ax\n\t"
    "tmp_calc:\n\t"
    "mov (%%cx), %%bp\n\t"
    "mov (%%dx), %%ax\n\t"
    "imul %%ax, %%bp\n\t"
    "add %%bp, (%%di)\n\t"
    "add $2, %%cx\n\t"
    "add $2, %%dx\n\t"
    "dec %%ax\n\t"
    "jnz tmp_calc\n\t"
    "mov (%%di), %%bp\n\t"
    "mov %1, %%ax\n\t"
    "mov %4, %%dx\n\t"
    "y_update:\n\t"
    "mov (%%cx), %%ax\n\t"
    "imul %%bp, %%ax\n\t"
    "add %%ax, (%%dx)\n\t"
    "add $2, %%cx\n\t"
    "add $2, %%dx\n\t"
    "dec %%ax\n\t"
    "jnz y_update\n\t"
    "mov %1, %%bx\n\t"
    "add %%bx, %%bx\n\t"
    "add %%bx, %%cx\n\t"
    "dec %%bp\n\t"
    "jnz outer\n\t"
    :
    : "m"(m), "m"(n), "m"(A), "m"(x), "m"(y), "m"(tmp)
    : "memory", "cc", "%ax", "%bx", "%cx", "%dx", "%si", "%di", "%bp"
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
