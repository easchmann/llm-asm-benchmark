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
      "  movw %5, %%cx\n\t"
      "  testw %%cx, %%cx\n\t"
      "  je 5f\n\t"
      "  movw %0, %%si\n\t"
      "  movw %4, %%di\n\t"
      "  xorw %%dx, %%dx\n\t"
      "1:\n\t"
      "  movw (%%si), %%bx\n\t"
      "  pushw %%si\n\t"
      "  pushw %%di\n\t"
      "  pushw %%dx\n\t"
      "  movw %2, %%si\n\t"
      "  movw %5, %%cx\n\t"
      "2:\n\t"
      "  movw (%%di), %%ax\n\t"
      "  mulw (%%si)\n\t"
      "  addw %%ax, %%bx\n\t"
      "  addw $2, %%di\n\t"
      "  addw $2, %%si\n\t"
      "  loop 2b\n\t"
      "  popw %%dx\n\t"
      "  popw %%di\n\t"
      "  addw %6, %%di\n\t"
      "  popw %%si\n\t"
      "  movw %%bx, (%%si)\n\t"
      "  addw $2, %%si\n\t"
      "  incw %%dx\n\t"
      "  cmpw %5, %%dx\n\t"
      "  jb 1b\n\t"
      "  movw %1, %%si\n\t"
      "  movw %4, %%di\n\t"
      "  xorw %%dx, %%dx\n\t"
      "3:\n\t"
      "  movw (%%si), %%bx\n\t"
      "  pushw %%si\n\t"
      "  pushw %%di\n\t"
      "  pushw %%dx\n\t"
      "  movw %3, %%si\n\t"
      "  movw %5, %%cx\n\t"
      "4:\n\t"
      "  movw (%%di), %%ax\n\t"
      "  mulw (%%si)\n\t"
      "  addw %%ax, %%bx\n\t"
      "  addw %6, %%di\n\t"
      "  addw $2, %%si\n\t"
      "  loop 4b\n\t"
      "  popw %%dx\n\t"
      "  popw %%di\n\t"
      "  addw $2, %%di\n\t"
      "  popw %%si\n\t"
      "  movw %%bx, (%%si)\n\t"
      "  addw $2, %%si\n\t"
      "  incw %%dx\n\t"
      "  cmpw %5, %%dx\n\t"
      "  jb 3b\n\t"
      "5:\n\t"
      :
      : "m"(x1), "m"(x2), "m"(y_1), "m"(y_2), "m"(A), "m"(nn), "m"(stride)
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
