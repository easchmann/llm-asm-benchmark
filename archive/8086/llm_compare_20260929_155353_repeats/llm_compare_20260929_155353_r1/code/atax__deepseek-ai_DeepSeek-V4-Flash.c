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
asm volatile (
    "    sub si, si\n"
    "L_Y_ZERO:\n"
    "    cmp si, %0\n"
    "    jge L_Y_ZERO_DONE\n"
    "    mov ax, %[y_ptr]\n"
    "    add ax, si\n"
    "    mov word ptr [bp+0x%c0], ax\n"
    "    mov word ptr [bp+0x%c2], ax\n"
    "    mov ax, 0\n"
    "    mov di, ax\n"
    "    mov [bp+0x%c4], si\n"
    "    mov [bp+0x%c6], si\n"
    "    mov ax, 0\n"
    "    mov [bp+0x%c8], ax\n"
    "    mov bx, [bp+0x%c0]\n"
    "    add bx, si\n"
    "    mov word ptr [bx], ax\n"
    "    inc si\n"
    "    jmp L_Y_ZERO\n"
    "L_Y_ZERO_DONE:\n"
    "    sub di, di\n"
    "L_I_LOOP:\n"
    "    cmp di, %1\n"
    "    jge L_I_LOOP_DONE\n"
    "    mov ax, %[tmp_ptr]\n"
    "    add ax, di\n"
    "    mov word ptr [bp+0x%c0], ax\n"
    "    mov word ptr [bp+0x%c2], ax\n"
    "    mov ax, 0\n"
    "    mov [bp+0x%c4], ax\n"
    "    mov si, 0\n"
    "L_J_TMP:\n"
    "    cmp si, %0\n"
    "    jge L_J_TMP_DONE\n"
    "    mov ax, %[A_ptr]\n"
    "    imul di, %2, 1\n"
    "    add ax, di\n"
    "    add ax, si\n"
    "    mov word ptr [bp+0x%c6], ax\n"
    "    mov bx, ax\n"
    "    mov ax, [bx]\n"
    "    mov word ptr [bp+0x%c8], ax\n"
    "    mov ax, %[x_ptr]\n"
    "    add ax, si\n"
    "    mov word ptr [bp+0x%ca], ax\n"
    "    mov bx, ax\n"
    "    mov ax, [bx]\n"
    "    imul word ptr [bp+0x%c8]\n"
    "    add [bp+0x%c4], ax\n"
    "    inc si\n"
    "    jmp L_J_TMP\n"
    "L_J_TMP_DONE:\n"
    "    mov ax, [bp+0x%c4]\n"
    "    mov bx, [bp+0x%c2]\n"
    "    mov [bx], ax\n"
    "    mov si, 0\n"
    "L_J_UPDATE:\n"
    "    cmp si, %0\n"
    "    jge L_J_UPDATE_DONE\n"
    "    mov bx, %[A_ptr]\n"
    "    imul ax, di, %2\n"
    "    add bx, ax\n"
    "    add bx, si\n"
    "    mov word ptr [bp+0x%cc], bx\n"
    "    mov ax, [bx]\n"
    "    mov word ptr [bp+0x%ce], ax\n"
    "    mov bx, %[y_ptr]\n"
    "    add bx, si\n"
    "    mov ax, [bx]\n"
    "    mov cx, [bp+0x%c4]\n"
    "    imul cx, [bp+0x%ce]\n"
    "    add ax, cx\n"
    "    mov [bx], ax\n"
    "    inc si\n"
    "    jmp L_J_UPDATE\n"
    "L_J_UPDATE_DONE:\n"
    "    inc di\n"
    "    jmp L_I_LOOP\n"
    "L_I_LOOP_DONE:\n"
    :
    : "i" (n), "i" (m), "i" (1),
      [A_ptr] "r" (A), [x_ptr] "r" (x), [y_ptr] "r" (y), [tmp_ptr] "r" (tmp)
    : "memory", "ax", "bx", "cx", "si", "di", "cc"
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
