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
  "	pushw %%bp\n"
  "	movw %%sp, %%bp\n"
  "	subw $16, %%sp\n"
  "	movw %0, -12(%%bp)\n"
  "	movw %1, -2(%%bp)\n"
  "	movw %2, -4(%%bp)\n"
  "	movw %3, -6(%%bp)\n"
  "	movw %4, -8(%%bp)\n"
  "	movw %5, -10(%%bp)\n"
  "	movw -12(%%bp), %%ax\n"
  "	addw %%ax, %%ax\n"
  "	movw %%ax, -14(%%bp)\n"
  "	movw -2(%%bp), %%ax\n"
  "	addw -14(%%bp), %%ax\n"
  "	movw %%ax, -16(%%bp)\n"
  "	movw -2(%%bp), %%si\n"
  "	movw -10(%%bp), %%bx\n"
  "1:\n"
  "	movw -6(%%bp), %%di\n"
  "	movw -12(%%bp), %%cx\n"
  "2:\n"
  "	movw (%%di), %%ax\n"
  "	mulw (%%bx)\n"
  "	addw (%%si), %%ax\n"
  "	movw %%ax, (%%si)\n"
  "	addw $2, %%di\n"
  "	addw $2, %%bx\n"
  "	loop 2b\n"
  "	addw $2, %%si\n"
  "	cmpw -16(%%bp), %%si\n"
  "	jne 1b\n"
  "	movw -4(%%bp), %%ax\n"
  "	addw -14(%%bp), %%ax\n"
  "	movw %%ax, -16(%%bp)\n"
  "	movw -4(%%bp), %%si\n"
  "	movw -10(%%bp), %%bx\n"
  "	subw %%dx, %%dx\n"
  "3:\n"
  "	movw -8(%%bp), %%di\n"
  "	movw -12(%%bp), %%cx\n"
  "4:\n"
  "	movw (%%di), %%ax\n"
  "	mulw (%%bx)\n"
  "	addw (%%si), %%ax\n"
  "	movw %%ax, (%%si)\n"
  "	addw $2, %%di\n"
  "	addw -14(%%bp), %%bx\n"
  "	loop 4b\n"
  "	addw $2, %%si\n"
  "	incw %%dx\n"
  "	movw -10(%%bp), %%bx\n"
  "	movw %%dx, %%cx\n"
  "	addw %%cx, %%cx\n"
  "	addw %%cx, %%bx\n"
  "	cmpw -16(%%bp), %%si\n"
  "	jne 3b\n"
  "	movw %%bp, %%sp\n"
  "	popw %%bp\n"
  : : "r"(n), "r"(x1), "r"(x2), "r"(y_1), "r"(y_2), "r"(A)
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
