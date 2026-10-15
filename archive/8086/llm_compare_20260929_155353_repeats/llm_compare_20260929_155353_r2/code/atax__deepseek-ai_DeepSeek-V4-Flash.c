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
    "push %%bp\n"
    "mov %%sp, %%bp\n"
    "push %%si\n"
    "push %%di\n"
    "push %%bx\n"
    "mov 4(%%bp), %%bx\n"
    "mov 6(%%bp), %%cx\n"
    "mov 8(%%bp), %%si\n"
    "mov 10(%%bp), %%di\n"
    "mov 12(%%bp), %%dx\n"
    "mov 14(%%bp), %%ax\n"

    /* Zero y[0..n-1] */
    "mov 8(%%bp), %%si\n"
    "xor %%di, %%di\n"
    "mov 6(%%bp), %%cx\n"
    "test %%cx, %%cx\n"
    "jle zero_done\n"
    "zero_loop:\n"
    "mov $0, (%%si)\n"
    "add $2, %%si\n"
    "inc %%di\n"
    "cmp %%di, %%cx\n"
    "jg zero_loop\n"
    "zero_done:\n"

    /* Outer loop i=0..m-1 */
    "xor %%bx, %%bx\n"
    "mov 4(%%bp), %%cx\n"
    "test %%cx, %%cx\n"
    "jle outer_done\n"
    "outer_loop:\n"
    /* tmp[i] = 0 */
    "mov 12(%%bp), %%dx\n"
    "mov $0, (%%dx, %%bx, 2)\n"

    /* First inner loop j=0..n-1: tmp[i] += A[i][j]*x[j] */
    "xor %%di, %%di\n"
    "mov 6(%%bp), %%cx\n"
    "test %%cx, %%cx\n"
    "jle inner1_done\n"
    "inner1_loop:\n"
    "mov 14(%%bp), %%ax\n"
    "mov 10(%%bp), %%si\n"
    "mov (%%ax, %%bx, 2), %%ax\n"
    "mov (%%si, %%di, 2), %%si\n"
    "imul %%si, %%ax\n"
    "mov 12(%%bp), %%dx\n"
    "add %%ax, (%%dx, %%bx, 2)\n"
    "inc %%di\n"
    "cmp %%di, %%cx\n"
    "jg inner1_loop\n"
    "inner1_done:\n"

    /* Second inner loop j=0..n-1: y[j] += A[i][j]*tmp[i] */
    "xor %%di, %%di\n"
    "mov 6(%%bp), %%cx\n"
    "test %%cx, %%cx\n"
    "jle inner2_done\n"
    "inner2_loop:\n"
    "mov 14(%%bp), %%ax\n"
    "mov 8(%%bp), %%si\n"
    "mov (%%ax, %%bx, 2), %%ax\n"
    "mov 12(%%bp), %%dx\n"
    "mov (%%dx, %%bx, 2), %%dx\n"
    "imul %%dx, %%ax\n"
    "add %%ax, (%%si, %%di, 2)\n"
    "inc %%di\n"
    "cmp %%di, %%cx\n"
    "jg inner2_loop\n"
    "inner2_done:\n"

    "inc %%bx\n"
    "cmp %%bx, 4(%%bp)\n"
    "jl outer_loop\n"
    "outer_done:\n"

    "pop %%bx\n"
    "pop %%di\n"
    "pop %%si\n"
    "pop %%bp\n"
    :
    :
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
