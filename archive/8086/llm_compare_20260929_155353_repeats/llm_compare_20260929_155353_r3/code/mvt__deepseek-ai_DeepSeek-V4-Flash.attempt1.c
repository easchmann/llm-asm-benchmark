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
asm(
    "mov %1, %%bx\n"            /* bx = n */
    "mov %2, %%si\n"            /* si = &A[0][0] */
    "mov %3, %%di\n"            /* di = &y_1[0] */
    "mov %4, %%bp\n"            /* bp = &x1[0] */
    "xor %%cx, %%cx\n"          /* i = 0 */
    "outer_loop_1:\n"
    "cmp %%cx, %%bx\n"
    "jge end_1\n"
    "xor %%dx, %%dx\n"          /* j = 0 */
    "xor %%ax, %%ax\n"          /* sum = 0 */
    "inner_loop_1:\n"
    "cmp %%dx, %%bx\n"
    "jge done_inner_1\n"
    "mov (%%si,%%dx,2), %%ax\n" /* A[i][j] */
    "mulw (%%di,%%dx,2)\n"      /* A[i][j] * y_1[j] */
    "add %%ax, %%ax\n"          /* adjust for 16-bit multiply result */
    "add %%dx, %%ax\n"          /* combine high and low */
    "add %%ax, %0\n"
    "inc %%dx\n"
    "jmp inner_loop_1\n"
    "done_inner_1:\n"
    "add %0, (%%bp,%%cx,2)\n"   /* x1[i] = x1[i] + sum */
    "inc %%cx\n"
    "add $%1, %%si\n"           /* advance to next row */
    "jmp outer_loop_1\n"
    "end_1:\n"
    "mov %1, %%bx\n"            /* bx = n */
    "mov %2, %%si\n"            /* si = &A[0][0] */
    "mov %4, %%di\n"            /* di = &y_2[0] */
    "mov %5, %%bp\n"            /* bp = &x2[0] */
    "xor %%cx, %%cx\n"          /* i = 0 */
    "outer_loop_2:\n"
    "cmp %%cx, %%bx\n"
    "jge end_2\n"
    "xor %%dx, %%dx\n"          /* j = 0 */
    "xor %%ax, %%ax\n"          /* sum = 0 */
    "inner_loop_2:\n"
    "cmp %%dx, %%bx\n"
    "jge done_inner_2\n"
    "mov (%%si,%%dx,2), %%ax\n" /* A[j][i] */
    "mulw (%%di,%%dx,2)\n"      /* A[j][i] * y_2[j] */
    "add %%ax, %%ax\n"          /* adjust */
    "add %%dx, %%ax\n"          /* combine */
    "add %%ax, %0\n"
    "inc %%dx\n"
    "jmp inner_loop_2\n"
    "done_inner_2:\n"
    "add %0, (%%bp,%%cx,2)\n"   /* x2[i] = x2[i] + sum */
    "inc %%cx\n"
    "add $%1, %%si\n"           /* advance to next row */
    "jmp outer_loop_2\n"
    "end_2:\n"
    :
    : "r"(n), "r"(A[0][0]), "r"(y_1[0]), "r"(y_2[0]), "r"(x1[0]), "r"(x2[0])
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
