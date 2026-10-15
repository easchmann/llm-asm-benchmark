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
asm volatile(
  "push si\n\t"
  "push di\n\t"
  "push bx\n\t"
  "push dx\n\t"
  "push ax\n\t"
  "mov cx, [bp+8]\n\t"
  "xor si, si\n\t"
  "mov ax, [bp+10]\n\t"
  "mov bx, [bp+18]\n\t"
  "mov dx, [bp+14]\n\t"
  "1:\n\t"
  "cmp si, cx\n\t"
  "jge 2f\n\t"
  "mov di, si\n\t"
  "add di, di\n\t"
  "add ax, di\n\t"
  "mov di, ax\n\t"
  "mov ax, si\n\t"
  "imul ax, cx\n\t"
  "shl ax, 1\n\t"
  "rcr dx, 1\n\t"
  "mov bx, [bp+18]\n\t"
  "add bx, ax\n\t"
  "adc bx, dx\n\t"
  "mov dx, [bp+14]\n\t"
  "xor di, di\n\t"
  "3:\n\t"
  "cmp di, cx\n\t"
  "jge 4f\n\t"
  "mov ax, [bx]\n\t"
  "imul ax, [dx]\n\t"
  "mov bx, [bp+10]\n\t"
  "mov cx, si\n\t"
  "add cx, cx\n\t"
  "add bx, cx\n\t"
  "mov cx, [bx]\n\t"
  "add cx, ax\n\t"
  "mov [bx], cx\n\t"
  "inc dx\n\t"
  "inc dx\n\t"
  "inc bx\n\t"
  "inc bx\n\t"
  "inc di\n\t"
  "jmp 3b\n\t"
  "4:\n\t"
  "inc si\n\t"
  "jmp 1b\n\t"
  "2:\n\t"
  "mov cx, [bp+8]\n\t"
  "xor si, si\n\t"
  "mov ax, [bp+12]\n\t"
  "mov bx, [bp+18]\n\t"
  "mov dx, [bp+16]\n\t"
  "5:\n\t"
  "cmp si, cx\n\t"
  "jge 6f\n\t"
  "mov di, si\n\t"
  "add di, di\n\t"
  "add ax, di\n\t"
  "mov di, ax\n\t"
  "mov di, si\n\t"
  "add di, di\n\t"
  "mov bx, [bp+18]\n\t"
  "add bx, di\n\t"
  "mov dx, [bp+16]\n\t"
  "xor di, di\n\t"
  "7:\n\t"
  "cmp di, cx\n\t"
  "jge 8f\n\t"
  "mov ax, [bx]\n\t"
  "imul ax, [dx]\n\t"
  "mov bx, [bp+12]\n\t"
  "mov cx, si\n\t"
  "add cx, cx\n\t"
  "add bx, cx\n\t"
  "mov cx, [bx]\n\t"
  "add cx, ax\n\t"
  "mov [bx], cx\n\t"
  "inc dx\n\t"
  "inc dx\n\t"
  "add bx, [bp+8]\n\t"
  "add bx, [bp+8]\n\t"
  "inc di\n\t"
  "jmp 7b\n\t"
  "8:\n\t"
  "inc si\n\t"
  "jmp 5b\n\t"
  "6:\n\t"
  "pop ax\n\t"
  "pop dx\n\t"
  "pop bx\n\t"
  "pop di\n\t"
  "pop si\n\t"
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
