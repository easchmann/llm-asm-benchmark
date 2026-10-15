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
asm volatile(
    /* Initialize s[0..m-1] to 0 */
    "movw %4, %%ax\n\t"          /* i = 0 */
    "1:\n\t"
    "cmpl %0, %%ax\n\t"          /* if i >= _PB_M, done */
    "jge 2f\n\t"
    "movw %%ax, %%bx\n\t"
    "shlw $1, %%bx\n\t"          /* i*2 for 16-bit DATA_TYPE */
    "movw %3, %%di\n\t"
    "addw %%bx, %%di\n\t"
    "movw $0, (%%di)\n\t"        /* s[i] = 0 */
    "incw %%ax\n\t"
    "jmp 1b\n"
    "2:\n\t"

    /* Main loop: for i = 0..n-1 */
    "movw $0, %%ax\n\t"          /* i = 0 */
    "3:\n\t"
    "cmpl %1, %%ax\n\t"          /* if i >= _PB_N, done */
    "jge 8f\n\t"

    /* q[i] = 0 */
    "movw %%ax, %%bx\n\t"
    "shlw $1, %%bx\n\t"
    "movw %5, %%di\n\t"
    "addw %%bx, %%di\n\t"
    "movw $0, (%%di)\n\t"

    /* for j = 0..m-1 */
    "movw $0, %%cx\n\t"          /* j = 0 */
    "4:\n\t"
    "cmpl %0, %%cx\n\t"          /* if j >= _PB_M, done */
    "jge 7f\n\t"

    /* Compute r[i] */
    "movw %%ax, %%bx\n\t"
    "shlw $1, %%bx\n\t"
    "movw %6, %%di\n\t"
    "addw %%bx, %%di\n\t"
    "movw (%%di), %%dx\n\t"      /* r[i] */

    /* Compute A[i][j] = A[i*m + j] */
    "movw %0, %%bx\n\t"
    "imulw %%ax, %%bx\n\t"       /* i*m */
    "addw %%cx, %%bx\n\t"        /* + j */
    "shlw $1, %%bx\n\t"          /* * sizeof(DATA_TYPE) */
    "movw %2, %%si\n\t"
    "addw %%bx, %%si\n\t"
    "movw (%%si), %%bx\n\t"      /* A[i][j] */

    /* s[j] += r[i] * A[i][j] */
    "movw %%cx, %%di\n\t"
    "shlw $1, %%di\n\t"
    "movw %3, %%di\n\t"
    "addw %%di, %%si\n\t"
    "movw %%dx, %%ax\n\t"
    "imulw %%bx, %%ax\n\t"
    "addw (%%si), %%ax\n\t"
    "movw %%ax, (%%si)\n\t"

    /* q[i] += A[i][j] * p[j] */
    "movw %%cx, %%di\n\t"
    "shlw $1, %%di\n\t"
    "movw %6, %%si\n\t"
    "addw %%di, %%si\n\t"
    "movw (%%si), %%ax\n\t"      /* p[j] */
    "imulw %%bx, %%ax\n\t"
    "movw %5, %%si\n\t"
    "movw %%ax, %%bx\n\t"
    "movw (%%si), %%ax\n\t"
    "addw %%bx, %%ax\n\t"
    "movw %%ax, (%%si)\n\t"

    "incw %%cx\n\t"
    "jmp 4b\n"
    "7:\n\t"
    "incw %%ax\n\t"
    "jmp 3b\n"
    "8:\n\t"
    :
    : "r" (_PB_M), "r" (_PB_N), "r" (A), "r" (s), "r" (p), "r" (q), "r" (r)
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
