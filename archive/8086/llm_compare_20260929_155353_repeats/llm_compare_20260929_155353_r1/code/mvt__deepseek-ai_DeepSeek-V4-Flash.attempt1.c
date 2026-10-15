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
__asm__ __volatile__ (
  "push si\n\t"
  "push di\n\t"
  "push bx\n\t"
  "push dx\n\t"
  "mov cx, %5\n\t"          /* n */
  "shl cx, 1\n\t"           /* n*2 for array offsets */
  "xor si, si\n\t"          /* i = 0 */
  "outer_loop1:\n\t"
  "mov ax, [%1+si]\n\t"     /* load x1[i] */
  "xor di, di\n\t"          /* j = 0 */
  "inner_loop1:\n\t"
  "mov bx, si\n\t"
  "shl bx, 1\n\t"
  "add bx, di\n\t"
  "mov dx, [%0+bx]\n\t"     /* A[i][j] */
  "imul word ptr [%3+di]\n\t" /* * y_1[j] */
  "add ax, dx\n\t"          /* accumulate */
  "add di, 2\n\t"
  "cmp di, cx\n\t"
  "jl inner_loop1\n\t"
  "mov [%1+si], ax\n\t"     /* store x1[i] */
  "add si, 2\n\t"
  "cmp si, cx\n\t"
  "jl outer_loop1\n\t"

  "xor si, si\n\t"          /* i = 0 for x2 loop */
  "outer_loop2:\n\t"
  "mov ax, [%2+si]\n\t"     /* load x2[i] */
  "xor di, di\n\t"          /* j = 0 */
  "inner_loop2:\n\t"
  "mov bx, di\n\t"
  "shl bx, 1\n\t"
  "add bx, si\n\t"
  "mov dx, [%0+bx]\n\t"     /* A[j][i] */
  "imul word ptr [%4+di]\n\t" /* * y_2[j] */
  "add ax, dx\n\t"
  "add di, 2\n\t"
  "cmp di, cx\n\t"
  "jl inner_loop2\n\t"
  "mov [%2+si], ax\n\t"     /* store x2[i] */
  "add si, 2\n\t"
  "cmp si, cx\n\t"
  "jl outer_loop2\n\t"

  "pop dx\n\t"
  "pop bx\n\t"
  "pop di\n\t"
  "pop si\n\t"
  :
  : "r" (A), "r" (x1), "r" (x2), "r" (y_1), "r" (y_2), "r" (n)
  : "memory", "ax", "cx"
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
