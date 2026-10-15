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
    "mov cx, %0\n\t"          /* cx = n */
    "mov si, %1\n\t"          /* si = x1 */
    "mov di, %2\n\t"          /* di = x2 */
    "mov bx, %3\n\t"          /* bx = y_1 */
    "mov bp, %4\n\t"          /* bp = y_2 */
    "mov dx, %5\n\t"          /* dx = A */
    "mov ax, cx\n\t"
    "shl ax, 1\n\t"           /* ax = n*2 stride */
    "push ax\n\t"
    "xor ax, ax\n\t"          /* ax = i */
"1:\n\t"
    "cmp cx, ax\n\t"
    "jge 3f"
    "push ax\n\t"
    "push si\n\t"
    "push bx\n\t"
    "push dx\n\t"
    "mov ax, [si]\n\t"
    "xor di, di\n\t"          /* di = acc low */
    "xor bp, bp\n\t"          /* bp = acc high */
    "xor cx, cx\n\t"
"2:\n\t"
    "cmp cx, %0\n\t"
    "jge 4f"
    "mov ax, [bx+cx*2]\n\t"
    "mul word ptr [dx+cx*2]\n\t" /* dx:ax = product */
    "add di, ax\n\t"
    "adc bp, dx\n\t"
    "inc cx\n\t"
    "jmp 2b"
"4:\n\t"
    "mov [si], di\n\t"
    "pop dx\n\t"
    "pop bx\n\t"
    "pop si\n\t"
    "pop ax\n\t"
    "add si, 2\n\t"
    "add dx, 2\n\t"
    "pop cx\n\t"
    "add dx, cx\n\t"
    "inc ax\n\t"
    "jmp 1b"
"3:\n\t"
    "mov ax, %5\n\t"          /* ax = A */
    "mov cx, %0\n\t"          /* cx = n */
    "xor di, di\n\t"          /* di = i */
"5:\n\t"
    "cmp cx, di\n\t"
    "jge 6f"
    "push di\n\t"
    "push si\n\t"
    "push bx\n\t"
    "push ax\n\t"
    "mov ax, [si]\n\t"
    "xor bp, bp\n\t"          /* bp = acc low */
    "xor cx, cx\n\t"          /* cx = j */
"7:\n\t"
    "cmp cx, %0\n\t"
    "jge 8f"
    "mov ax, [bx+cx*2]\n\t"
    "mul word ptr [sp+8]\n\t" /* A ptr saved earlier */
    "add bp, ax\n\t"
    "adc dx, dx\n\t"
    "inc cx\n\t"
    "jmp 7b"
"8:\n\t"
    "mov [si], bp\n\t"
    "pop ax\n\t"
    "pop bx\n\t"
    "pop si\n\t"
    "pop di\n\t"
    "add si, 2\n\t"
    "add ax, 2\n\t"
    "inc di\n\t"
    "jmp 5b"
"6:\n\t"
    : "+r" (A), "+r" (x1), "+r" (x2)
    : "r" (n), "r" (y_1), "r" (y_2)
    : "ax", "bx", "cx", "dx", "si", "di", "memory"
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
