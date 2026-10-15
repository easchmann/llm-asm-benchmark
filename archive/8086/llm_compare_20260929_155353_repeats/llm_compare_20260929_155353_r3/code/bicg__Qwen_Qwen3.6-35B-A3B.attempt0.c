/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* bicg.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
#include "bicg.h"


/* Array initialization. */
static
void init_array (int m, int n,
		 DATA_TYPE POLYBENCH_2D(A,N,M,n,m),
		 DATA_TYPE POLYBENCH_1D(r,N,n),
		 DATA_TYPE POLYBENCH_1D(p,M,m))
{
  int i, j;

  for (i = 0; i < m; i++)
    p[i] = (DATA_TYPE)(i % m) / m;
  for (i = 0; i < n; i++) {
    r[i] = (DATA_TYPE)(i % n) / n;
    for (j = 0; j < m; j++)
      A[i][j] = (DATA_TYPE) (i*(j+1) % n)/n;
  }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int m, int n,
		 DATA_TYPE POLYBENCH_1D(s,M,m),
		 DATA_TYPE POLYBENCH_1D(q,N,n))

{
  int i;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("s");
  for (i = 0; i < m; i++) {
    if (i % 20 == 0) fprintf (POLYBENCH_DUMP_TARGET, "\n");
    fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, s[i]);
  }
  POLYBENCH_DUMP_END("s");
  POLYBENCH_DUMP_BEGIN("q");
  for (i = 0; i < n; i++) {
    if (i % 20 == 0) fprintf (POLYBENCH_DUMP_TARGET, "\n");
    fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, q[i]);
  }
  POLYBENCH_DUMP_END("q");
  POLYBENCH_DUMP_FINISH;
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_bicg(int m, int n,
		 DATA_TYPE POLYBENCH_2D(A,N,M,n,m),
		 DATA_TYPE POLYBENCH_1D(s,M,m),
		 DATA_TYPE POLYBENCH_1D(q,N,n),
		 DATA_TYPE POLYBENCH_1D(p,M,m),
		 DATA_TYPE POLYBENCH_1D(r,N,n))
{
  int i, j;

#pragma scop
__asm__ volatile (
  "push bx\n\t"
  "push cx\n\t"
  "push dx\n\t"
  "push di\n\t"
  "push si\n\t"
  "push ax\n\t"
  "mov si, [bp+4]\n\t"
  "mov di, [bp+6]\n\t"
  "mov ax, [bp+8]\n\t"
  "mov bx, [bp+10]\n\t"
  "mov cx, [bp+12]\n\t"
  "mov dx, [bp+14]\n\t"
  "xor si, si\n\t"
"1:\n\t"
  "cmp si, [bp+4]\n\t"
  "jae 2f\n\t"
  "mov dword ptr [bx + si*4], 0\n\t"
  "inc si\n\t"
  "jmp 1b\n\t"
"2:\n\t"
  "xor di, di\n\t"
"3:\n\t"
  "cmp di, [bp+6]\n\t"
  "jae 6f\n\t"
  "mov dword ptr [cx + di*4], 0\n\t"
  "fld dword ptr [cx + di*4]\n\t"
  "mov ax, [bp+8]\n\t"
  "mov ax, di\n\t"
  "mul word ptr [bp+4]\n\t"
  "shl ax, 2\n\t"
  "add ax, [bp+8]\n\t"
  "xor si, si\n\t"
"4:\n\t"
  "cmp si, [bp+4]\n\t"
  "jae 5f\n\t"
  "fld dword ptr [ax + si*4]\n\t"
  "fld dword ptr [bp+16 + di*4]\n\t"
  "fmulp st(1), st(0)\n\t"
  "fadd dword ptr [bx + si*4]\n\t"
  "fstp dword ptr [bx + si*4]\n\t"
  "fld dword ptr [ax + si*4]\n\t"
  "fmul st(0), dword ptr [dx + si*4]\n\t"
  "faddp st(1), st(0)\n\t"
  "fstp dword ptr [cx + di*4]\n\t"
  "fld dword ptr [cx + di*4]\n\t"
  "inc si\n\t"
  "jmp 4b\n\t"
"5:\n\t"
  "inc di\n\t"
  "jmp 3b\n\t"
"6:\n\t"
  "pop ax\n\t"
  "pop si\n\t"
  "pop di\n\t"
  "pop dx\n\t"
  "pop cx\n\t"
  "pop bx\n\t"
  :
  :
  : "memory", "cc", "st", "ax", "bx", "cx", "dx", "di", "si"
);
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int n = N;
  int m = M;

  /* Variable declaration/allocation. */
  POLYBENCH_2D_ARRAY_DECL(A, DATA_TYPE, N, M, n, m);
  POLYBENCH_1D_ARRAY_DECL(s, DATA_TYPE, M, m);
  POLYBENCH_1D_ARRAY_DECL(q, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(p, DATA_TYPE, M, m);
  POLYBENCH_1D_ARRAY_DECL(r, DATA_TYPE, N, n);

  /* Initialize array(s). */
  init_array (m, n,
	      POLYBENCH_ARRAY(A),
	      POLYBENCH_ARRAY(r),
	      POLYBENCH_ARRAY(p));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_bicg (m, n,
	       POLYBENCH_ARRAY(A),
	       POLYBENCH_ARRAY(s),
	       POLYBENCH_ARRAY(q),
	       POLYBENCH_ARRAY(p),
	       POLYBENCH_ARRAY(r));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(m, n, POLYBENCH_ARRAY(s), POLYBENCH_ARRAY(q)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(s);
  POLYBENCH_FREE_ARRAY(q);
  POLYBENCH_FREE_ARRAY(p);
  POLYBENCH_FREE_ARRAY(r);

  return 0;
}
