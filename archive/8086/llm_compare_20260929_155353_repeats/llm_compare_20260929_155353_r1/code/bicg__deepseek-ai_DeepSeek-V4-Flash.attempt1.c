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
__asm__ __volatile__ (
    "mov si, %4\n\t"
    "mov di, %5\n\t"
    "mov ax, %2\n\t"
    "xor bx, bx\n\t"
    "s_init_loop:\n\t"
    "cmp bx, ax\n\t"
    "jge s_init_done\n\t"
    "mov WORD PTR [di+bx*2], 0\n\t"
    "inc bx\n\t"
    "jmp s_init_loop\n\t"
    "s_init_done:\n\t"
    "mov ax, %3\n\t"
    "xor bx, bx\n\t"
    "q_outer_loop:\n\t"
    "cmp bx, ax\n\t"
    "jge q_outer_done\n\t"
    "mov WORD PTR [si+bx*2], 0\n\t"
    "mov cx, %2\n\t"
    "xor dx, dx\n\t"
    "q_inner_loop:\n\t"
    "cmp dx, cx\n\t"
    "jge q_inner_done\n\t"
    "mov bp, %0\n\t"
    "mov ax, dx\n\t"
    "mul %1\n\t"
    "add ax, dx\n\t"
    "shl ax, 1\n\t"
    "add bp, ax\n\t"
    "mov ax, [bp]\n\t"
    "mov bp, %5\n\t"
    "mov si, %7\n\t"
    "mov bx, dx\n\t"
    "mov cx, [si+bx*2]\n\t"
    "mul cx\n\t"
    "add [bp+dx*2], ax\n\t"
    "mov bp, %0\n\t"
    "mov si, %6\n\t"
    "mov ax, [bp]\n\t"
    "mul WORD PTR [si+dx*2]\n\t"
    "add [di+bx*2], ax\n\t"
    "inc dx\n\t"
    "jmp q_inner_loop\n\t"
    "q_inner_done:\n\t"
    "inc bx\n\t"
    "jmp q_outer_loop\n\t"
    "q_outer_done:\n\t"
    :
    : "m" (*(DATA_TYPE (*)[m])A), "r" (n), "r" (m), "r" (n), "r" (q), "r" (s), "r" (p), "r" (r)
    : "cc", "memory", "ax", "bx", "cx", "dx", "si", "di", "bp"
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
