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
    "mov $0, %%si\n\t"
    "1:\n\t"
    "cmp %4, %%si\n\t"
    "jge 2f\n\t"
    "mov $0, (%%di, %%si, 2)\n\t"
    "inc %%si\n\t"
    "jmp 1b\n\t"
    "2:\n\t"
    "mov $0, %%si\n\t"
    "3:\n\t"
    "cmp %3, %%si\n\t"
    "jge 8f\n\t"
    "mov $0, (%%bx, %%si, 2)\n\t"
    "mov $0, %%dx\n\t"
    "4:\n\t"
    "cmp %4, %%dx\n\t"
    "jge 6f\n\t"
    "push %%si\n\t"
    "push %%dx\n\t"
    "mov %%si, %%ax\n\t"
    "imul %4, %%ax\n\t"
    "add %%dx, %%ax\n\t"
    "mov (%%ax, %%si, 2), %%cx\n\t"
    "pop %%dx\n\t"
    "push %%dx\n\t"
    "mov (%%cx, %%dx, 2), %%cx\n\t"
    "pop %%dx\n\t"
    "pop %%si\n\t"
    "push %%dx\n\t"
    "push %%cx\n\t"
    "mov (%%bx, %%si, 2), %%cx\n\t"
    "add %%cx, (%%sp)\n\t"
    "pop %%cx\n\t"
    "pop %%dx\n\t"
    "inc %%dx\n\t"
    "jmp 4b\n\t"
    "6:\n\t"
    "mov (%%bx, %%si, 2), %%cx\n\t"
    "mov $0, %%dx\n\t"
    "7:\n\t"
    "cmp %4, %%dx\n\t"
    "jge 5f\n\t"
    "push %%si\n\t"
    "push %%dx\n\t"
    "mov %%si, %%ax\n\t"
    "imul %4, %%ax\n\t"
    "add %%dx, %%ax\n\t"
    "mov (%%ax, %%si, 2), %%cx\n\t"
    "pop %%dx\n\t"
    "push %%dx\n\t"
    "mov (%%cx, %%dx, 2), %%cx\n\t"
    "pop %%dx\n\t"
    "pop %%si\n\t"
    "push %%dx\n\t"
    "push %%cx\n\t"
    "push %%si\n\t"
    "mov (%%bx, %%si, 2), %%cx\n\t"
    "mov (%%di, %%dx, 2), %%si\n\t"
    "add %%cx, %%si\n\t"
    "mov %%si, (%%di, %%dx, 2)\n\t"
    "pop %%si\n\t"
    "pop %%cx\n\t"
    "pop %%dx\n\t"
    "inc %%dx\n\t"
    "jmp 7b\n\t"
    "5:\n\t"
    "inc %%si\n\t"
    "jmp 3b\n\t"
    "8:\n\t"
    :
    : "a" (A), "b" (tmp), "d" (y), "c" (m), "S" (n)
    : "memory"
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
