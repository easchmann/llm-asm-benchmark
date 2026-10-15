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
asm volatile (
  "mov %0, %%cx\n\t"
  "xor %%dx, %%dx\n\t"
  "1:\n\t"
  "cmp %%cx, %%dx\n\t"
  "jge 2f\n\t"
  "mov %1, %%si\n\t"
  "add %%dx, %%si\n\t"
  "add %%dx, %%si\n\t"
  "mov %5, %%bp\n\t"
  "add %%dx, %%bp\n\t"
  "add %%dx, %%bp\n\t"
  "mov %3, %%di\n\t"
  "xor %%bx, %%bx\n\t"
  "3:\n\t"
  "cmp %%cx, %%bx\n\t"
  "jge 4f\n\t"
  "mov %%bp, %%ax\n\t"
  "add %%bx, %%ax\n\t"
  "add %%bx, %%ax\n\t"
  "mov (%%ax), %%ax\n\t"
  "mov %%di, %%ax\n\t"
  "add %%bx, %%ax\n\t"
  "add %%bx, %%ax\n\t"
  "mov (%%ax), %%ax\n\t"
  "mul %%ax\n\t"
  "add %%ax, (%%si)\n\t"
  "inc %%bx\n\t"
  "jmp 3b\n\t"
  "4:\n\t"
  "inc %%dx\n\t"
  "jmp 1b\n\t"
  "2:\n\t"
  "xor %%dx, %%dx\n\t"
  "mov %0, %%cx\n\t"
  "mov %0, %%bp\n\t"
  "shl %%bp, %%bp\n\t"
  "mov %5, %%di\n\t"
  "mov %2, %%si\n\t"
  "5:\n\t"
  "cmp %%cx, %%dx\n\t"
  "jge 6f\n\t"
  "add %%dx, %%si\n\t"
  "add %%dx, %%si\n\t"
  "mov %%di, %%ax\n\t"
  "add %%dx, %%ax\n\t"
  "add %%dx, %%ax\n\t"
  "xor %%bx, %%bx\n\t"
  "7:\n\t"
  "cmp %%cx, %%bx\n\t"
  "jge 8f\n\t"
  "mov %%ax, %%dx\n\t"
  "mov %%bx, %%ax\n\t"
  "mul %%bp\n\t"
  "add %%ax, %%dx\n\t"
  "mov (%%dx), %%ax\n\t"
  "mov %4, %%di\n\t"
  "add %%bx, %%di\n\t"
  "add %%bx, %%di\n\t"
  "mov (%%di), %%di\n\t"
  "mul %%di\n\t"
  "add %%ax, (%%si)\n\t"
  "inc %%bx\n\t"
  "jmp 7b\n\t"
  "8:\n\t"
  "inc %%dx\n\t"
  "jmp 5b\n\t"
  "6:\n\t"
  :
  : "r"(n), "r"(x1), "r"(x2), "r"(y_1), "r"(y_2), "r"(A)
  : "ax", "bx", "cx", "dx", "si", "di", "bp", "memory"
);
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
