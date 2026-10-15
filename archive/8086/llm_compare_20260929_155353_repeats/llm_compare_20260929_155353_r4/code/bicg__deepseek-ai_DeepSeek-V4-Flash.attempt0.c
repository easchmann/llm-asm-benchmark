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
    "mov $0, %%bx\n"
    "mov %2, %%cx\n"        // _PB_M
    "mov %3, %%dx\n"        // _PB_N
    "xor %%si, %%si\n"
    "s_loop:\n"
    "cmp %%cx, %%si\n"
    "jge s_done\n"
    "mov %1, %%ax\n"
    "add %%si, %%ax\n"
    "movw $0, (%%ax)\n"
    "inc %%si\n"
    "jmp s_loop\n"
    "s_done:\n"
    "xor %%si, %%si\n"
    "q_outer:\n"
    "cmp %%dx, %%si\n"
    "jge q_done\n"
    "mov %4, %%ax\n"
    "add %%si, %%ax\n"
    "movw $0, (%%ax)\n"
    "xor %%di, %%di\n"
    "q_inner:\n"
    "cmp %%cx, %%di\n"
    "jge q_inner_done\n"
    // s[j] += r[i] * A[i][j]
    "mov %0, %%ax\n"
    "imul %%si, %%ax\n"
    "add %%di, %%ax\n"
    "mov (%%ax), %%ax\n"
    "mov %5, %%bx\n"
    "add %%si, %%bx\n"
    "mov (%%bx), %%bx\n"
    "imul %%bx, %%ax\n"
    "mov %1, %%bx\n"
    "add %%di, %%bx\n"
    "add %%ax, (%%bx)\n"
    // q[i] += A[i][j] * p[j]
    "mov %0, %%ax\n"
    "imul %%si, %%ax\n"
    "add %%di, %%ax\n"
    "mov (%%ax), %%ax\n"
    "mov %6, %%bx\n"
    "add %%di, %%bx\n"
    "mov (%%bx), %%bx\n"
    "imul %%bx, %%ax\n"
    "mov %4, %%bx\n"
    "add %%si, %%bx\n"
    "add %%ax, (%%bx)\n"
    "inc %%di\n"
    "jmp q_inner\n"
    "q_inner_done:\n"
    "inc %%si\n"
    "jmp q_outer\n"
    "q_done:\n"
    :
    : "r"(A), "r"(s), "r"(m), "r"(n), "r"(q), "r"(r), "r"(p)
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
