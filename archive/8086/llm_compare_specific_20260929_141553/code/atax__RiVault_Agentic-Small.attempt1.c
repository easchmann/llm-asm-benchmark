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
    "movw %4, %%si\n\t"
    "movw %1, %%ax\n\t"
"0:\n\t"
    "movw $0, (%%si)\n\t"
    "addw $2, %%si\n\t"
    "decw %%ax\n\t"
    "jnz 0b\n\t"
    "movw %4, %%si\n\t"
    "movw %0, %%bx\n\t"
"1:\n\t"
    "movw $0, (%%di)\n\t"
    "movw %%di, %%si\n\t"
    "movw %1, %%cx\n\t"
    "movw %2, %%dx\n\t"
    "movw %3, %%bp\n\t"
    "movw $0, %%ax\n\t"
"2:\n\t"
    "movw (%%dx), %%di\n\t"
    "movw (%%bp), %%bx\n\t"
    "imulw %%bx, %%di\n\t"
    "addw %%di, %%ax\n\t"
    "addw $2, %%dx\n\t"
    "addw $2, %%bp\n\t"
    "decw %%cx\n\t"
    "jnz 2b\n\t"
    "movw %%ax, (%%si)\n\t"
    "movw (%%si), %%ax\n\t"
    "movw %1, %%cx\n\t"
    "movw %2, %%dx\n\t"
    "movw %4, %%di\n\t"
"3:\n\t"
    "movw (%%dx), %%bp\n\t"
    "imulw %%ax, %%bp\n\t"
    "addw %%bp, (%%di)\n\t"
    "addw $2, %%dx\n\t"
    "addw $2, %%di\n\t"
    "decw %%cx\n\t"
    "jnz 3b\n\t"
    "movw %1, %%ax\n\t"
    "shlw $1, %%ax\n\t"
    "addw %%ax, %%dx\n\t"
    "addw $2, %%si\n\t"
    "decw %%bx\n\t"
    "jnz 1b\n\t"
    :
    : "r"(m), "r"(n), "r"(A), "r"(x), "r"(y), "r"(tmp)
    : "ax", "bx", "cx", "dx", "si", "di", "bp", "memory"
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
