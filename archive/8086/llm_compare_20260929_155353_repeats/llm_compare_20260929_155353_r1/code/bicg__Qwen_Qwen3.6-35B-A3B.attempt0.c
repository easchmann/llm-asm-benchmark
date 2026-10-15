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
asm volatile (
"mov cx, [bp+8]\n\t"
"mov di, [bp+14]\n\t"
"xor ax, ax\n\t"
"0:\n\t"
"mov dword ptr [di], ax\n\t"
"add di, 4\n\t"
"dec cx\n\t"
"jnz 0b\n\t"
"mov dx, [bp+10]\n\t"
"xor si, si\n\t"
"1:\n\t"
"push bx\n\t"
"mov bx, si\n\t"
"shl bx, 2\n\t"
"add bx, [bp+16]\n\t"
"fldz\n\t"
"fstp dword ptr [bx]\n\t"
"mov ax, [bp+20]\n\t"
"mov cx, si\n\t"
"shl cx, 2\n\t"
"add ax, cx\n\t"
"fld dword ptr [ax]\n\t"
"mov bx, [bp+14]\n\t"
"mov ax, [bp+12]\n\t"
"mov cx, [bp+8]\n\t"
"shl cx, 2\n\t"
"mov dx, si\n\t"
"shl dx, 2\n\t"
"mul cx\n\t"
"mov cx, ax\n\t"
"mov bx, dx\n\t"
"xor si, si\n\t"
"2:\n\t"
"push cx\n\t"
"push bx\n\t"
"mov ax, si\n\t"
"shl ax, 2\n\t"
"add ax, cx\n\t"
"adc ax, bx\n\t"
"push ax\n\t"
"mov ax, si\n\t"
"shl ax, 2\n\t"
"add ax, [bp+14]\n\t"
"fld dword ptr [ax]\n\t"
"pop ax\n\t"
"mov cx, [bp+12]\n\t"
"add cx, ax\n\t"
"fld dword ptr [cx]\n\t"
"fmul st(0), st(1)\n\t"
"faddp st(1), st(0)\n\t"
"pop ax\n\t"
"add ax, [bp+14]\n\t"
"fstp dword ptr [ax]\n\t"
"pop bx\n\t"
"pop cx\n\t"
"mov ax, si\n\t"
"shl ax, 2\n\t"
"add ax, [bp+18]\n\t"
"fld dword ptr [ax]\n\t"
"pop ax\n\t"
"mov cx, [bp+12]\n\t"
"add cx, ax\n\t"
"fld dword ptr [cx]\n\t"
"fmul st(0), st(1)\n\t"
"faddp st(1), st(0)\n\t"
"pop ax\n\t"
"add ax, [bp+16]\n\t"
"fstp dword ptr [ax]\n\t"
"inc si\n\t"
"cmp si, [bp+8]\n\t"
"jl 2b\n\t"
"pop bx\n\t"
"inc si\n\t"
"cmp si, [bp+10]\n\t"
"jl 1b\n\t"
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
