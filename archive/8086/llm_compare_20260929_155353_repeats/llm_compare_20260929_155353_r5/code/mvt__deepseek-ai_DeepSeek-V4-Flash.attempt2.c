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
"    mov cx, %[n]\n"
"    xor si, si\n"
"outer_loop_1:\n"
"    xor di, di\n"
"    push si\n"
"    push cx\n"
"    mov cx, %[n]\n"
"inner_loop_1:\n"
"    mov ax, %[n]\n"
"    mul di\n"
"    add ax, si\n"
"    shl ax, 1\n"
"    mov bx, ax\n"
"    mov ax, [%[A]+bx]\n"
"    shl si, 1\n"
"    mov dx, [%[y1]+si]\n"
"    shr si, 1\n"
"    imul dx\n"
"    shl si, 1\n"
"    mov bx, [%[x1]+si]\n"
"    shr si, 1\n"
"    add bx, ax\n"
"    shl si, 1\n"
"    mov [%[x1]+si], bx\n"
"    shr si, 1\n"
"    pop si\n"
"    push si\n"
"    inc di\n"
"    cmp di, cx\n"
"    jne inner_loop_1\n"
"    pop cx\n"
"    pop si\n"
"    inc si\n"
"    cmp si, cx\n"
"    jne outer_loop_1\n"
"    mov cx, %[n]\n"
"    xor si, si\n"
"outer_loop_2:\n"
"    xor di, di\n"
"    push si\n"
"    push cx\n"
"    mov cx, %[n]\n"
"inner_loop_2:\n"
"    mov ax, %[n]\n"
"    mul si\n"
"    add ax, di\n"
"    shl ax, 1\n"
"    mov bx, ax\n"
"    mov ax, [%[A]+bx]\n"
"    shl di, 1\n"
"    mov dx, [%[y2]+di]\n"
"    shr di, 1\n"
"    imul dx\n"
"    shl di, 1\n"
"    mov bx, [%[x2]+di]\n"
"    shr di, 1\n"
"    add bx, ax\n"
"    shl di, 1\n"
"    mov [%[x2]+di], bx\n"
"    shr di, 1\n"
"    pop di\n"
"    push di\n"
"    inc di\n"
"    cmp di, cx\n"
"    jne inner_loop_2\n"
"    pop cx\n"
"    pop di\n"
"    inc si\n"
"    cmp si, cx\n"
"    jne outer_loop_2\n"
:
: [A] "m" (*(DATA_TYPE (*)[N][N])A), [x1] "m" (*(DATA_TYPE (*)[N])x1), [x2] "m" (*(DATA_TYPE (*)[N])x2), [y1] "m" (*(DATA_TYPE (*)[N])y_1), [y2] "m" (*(DATA_TYPE (*)[N])y_2), [n] "r" ((unsigned int)n)
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
