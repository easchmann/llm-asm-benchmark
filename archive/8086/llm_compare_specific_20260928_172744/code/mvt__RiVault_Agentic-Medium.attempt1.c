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
        "        movw %4, %%ax\n"
        "        testw %%ax, %%ax\n"
        "        jle 2f\n"
        "        subw $4, %%sp\n"
        "        movw $0, -2(%%bp)\n"
        "1:\n"
        "        movw -2(%%bp), %%ax\n"
        "        movw %%ax, %%bx\n"
        "        imulw %4, %%bx\n"
        "        addw %6, %%bx\n"
        "        movw (%%bx), %%cx\n"
        "        movw $0, -4(%%bp)\n"
        "3:\n"
        "        movw -4(%%bp), %%ax\n"
        "        movw %%ax, %%di\n"
        "        movw -2(%%bp), %%bx\n"
        "        movw %%bx, %%si\n"
        "        imulw %4, %%bx\n"
        "        addw %%di, %%bx\n"
        "        movw %5, %%si\n"
        "        shlw $1, %%bx\n"
        "        addw %%bx, %%si\n"
        "        movw (%%si), %%ax\n"
        "        movw -4(%%bp), %%bx\n"
        "        shlw $1, %%bx\n"
        "        addw %7, %%bx\n"
        "        movw (%%bx), %%dx\n"
        "        imulw %%dx, %%ax\n"
        "        addw %%ax, %%cx\n"
        "        incw -4(%%bp)\n"
        "        movw -4(%%bp), %%ax\n"
        "        cmpw %4, %%ax\n"
        "        jl 3b\n"
        "        movw -2(%%bp), %%bx\n"
        "        shlw $1, %%bx\n"
        "        addw %6, %%bx\n"
        "        movw %%cx, (%%bx)\n"
        "        incw -2(%%bp)\n"
        "        movw -2(%%bp), %%ax\n"
        "        cmpw %4, %%ax\n"
        "        jl 1b\n"
        "        movw $0, -2(%%bp)\n"
        "4:\n"
        "        movw -2(%%bp), %%ax\n"
        "        movw %%ax, %%bx\n"
        "        shlw $1, %%bx\n"
        "        addw %8, %%bx\n"
        "        movw (%%bx), %%cx\n"
        "        movw $0, -4(%%bp)\n"
        "5:\n"
        "        movw -4(%%bp), %%ax\n"
        "        movw %%ax, %%di\n"
        "        movw %%di, %%bx\n"
        "        imulw %4, %%bx\n"
        "        addw -2(%%bp), %%bx\n"
        "        movw %5, %%si\n"
        "        shlw $1, %%bx\n"
        "        addw %%bx, %%si\n"
        "        movw (%%si), %%ax\n"
        "        movw -4(%%bp), %%bx\n"
        "        shlw $1, %%bx\n"
        "        addw %9, %%bx\n"
        "        movw (%%bx), %%dx\n"
        "        imulw %%dx, %%ax\n"
        "        addw %%ax, %%cx\n"
        "        incw -4(%%bp)\n"
        "        movw -4(%%bp), %%ax\n"
        "        cmpw %4, %%ax\n"
        "        jl 5b\n"
        "        movw -2(%%bp), %%bx\n"
        "        shlw $1, %%bx\n"
        "        addw %8, %%bx\n"
        "        movw %%cx, (%%bx)\n"
        "        incw -2(%%bp)\n"
        "        movw -2(%%bp), %%ax\n"
        "        cmpw %4, %%ax\n"
        "        jl 4b\n"
        "        addw $4, %%sp\n"
        "2:\n"
        :
        : "r" (n), "r" (A), "r" (x1), "r" (x2), "r" (y_1), "r" (y_2),
          "m" (n), "m" (A[0][0]), "m" (x1[0]), "m" (x2[0]), "m" (y_1[0]), "m" (y_2[0])
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
