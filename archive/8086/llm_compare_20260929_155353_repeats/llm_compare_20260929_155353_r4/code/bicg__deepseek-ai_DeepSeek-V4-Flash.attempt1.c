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
    "push %%bp\n"
    "movw $0, %%ax\n"
    "movw %2, %%cx\n"        // _PB_M
    "movw %3, %%dx\n"        // _PB_N
    "xorw %%si, %%si\n"
    "s_loop:\n"
    "cmpw %%cx, %%si\n"
    "jge s_done\n"
    "movw %1, %%ax\n"
    "addw %%si, %%ax\n"
    "movw $0, (%%ax)\n"
    "incw %%si\n"
    "jmp s_loop\n"
    "s_done:\n"
    "xorw %%si, %%si\n"
    "q_outer:\n"
    "cmpw %%dx, %%si\n"
    "jge q_done\n"
    "movw %4, %%ax\n"
    "addw %%si, %%ax\n"
    "movw $0, (%%ax)\n"
    "xorw %%di, %%di\n"
    "q_inner:\n"
    "cmpw %%cx, %%di\n"
    "jge q_inner_done\n"
    // s[j] += r[i] * A[i][j]
    "movw %0, %%ax\n"
    "mulw %%si\n"
    "addw %%di, %%ax\n"
    "movw (%%ax), %%ax\n"
    "movw %5, %%bx\n"
    "addw %%si, %%bx\n"
    "movw (%%bx), %%bx\n"
    "imulw %%bx, %%ax\n"
    "movw %1, %%bx\n"
    "addw %%di, %%bx\n"
    "addw %%ax, (%%bx)\n"
    // q[i] += A[i][j] * p[j]
    "movw %0, %%ax\n"
    "mulw %%si\n"
    "addw %%di, %%ax\n"
    "movw (%%ax), %%ax\n"
    "movw %6, %%bx\n"
    "addw %%di, %%bx\n"
    "movw (%%bx), %%bx\n"
    "imulw %%bx, %%ax\n"
    "movw %4, %%bx\n"
    "addw %%si, %%bx\n"
    "addw %%ax, (%%bx)\n"
    "incw %%di\n"
    "jmp q_inner\n"
    "q_inner_done:\n"
    "incw %%si\n"
    "jmp q_outer\n"
    "q_done:\n"
    "pop %%bp\n"
    :
    : "rm"(A), "rm"(s), "rm"(m), "rm"(n), "rm"(q), "rm"(r), "rm"(p)
    : "ax", "bx", "cx", "dx", "si", "di", "memory"
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
