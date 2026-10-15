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
  "movw %3, %%cx\n\t"
  "movw %4, %%dx\n\t"
  "movw %5, %%si\n\t"
  "movw %6, %%di\n\t"
  "movw %2, %%bp\n\t"
  "movw %0, %%ax\n\t"
  "pushw %%ax\n\t"
  "xorw %%ax, %%ax\n\t"
  "xorw %%si, %%si\n\t"
"1:\n\t"
  "cmpw %%si, (%%sp)\n\t"
  "jge 2f\n\t"
  "movw %%ax, (%%cx,%%si,2)\n\t"
  "incw %%si\n\t"
  "jmp 1b\n"
"2:\n\t"
  "popw %%ax\n\t"
  "xorw %%si, %%si\n\t"
"3:\n\t"
  "cmpw %%si, %1\n\t"
  "jge 4f\n\t"
  "pushw %%ax\n\t"
  "xorw %%ax, %%ax\n\t"
  "movw %%ax, (%%dx,%%si,2)\n\t"
  "popw %%ax\n\t"
  "movw %0, %%ax\n\t"
  "pushw %%ax\n\t"
  "xorw %%di, %%di\n\t"
"5:\n\t"
  "cmpw %%di, (%%sp)\n\t"
  "jge 6f\n\t"
  "movw %%si, %%bx\n\t"
  "imulw (%%sp), %%bx\n\t"
  "addw %%di, %%bx\n\t"
  "imulw $2, %%bx\n\t"
  "movw (%%bp,%%bx), %%ax\n\t"
  "imulw (%%di,%%si,2), %%ax\n\t"
  "addw %%ax, (%%cx,%%di,2)\n\t"
  "movw %%si, %%bx\n\t"
  "imulw (%%sp), %%bx\n\t"
  "addw %%di, %%bx\n\t"
  "imulw $2, %%bx\n\t"
  "movw (%%bp,%%bx), %%ax\n\t"
  "imulw (%%si,%%di,2), %%ax\n\t"
  "addw %%ax, (%%dx,%%si,2)\n\t"
  "incw %%di\n\t"
  "jmp 5b\n"
"6:\n\t"
  "popw %%ax\n\t"
  "incw %%si\n\t"
  "jmp 3b\n"
"4:\n\t"
  :
  : "r" (m), "r" (n), "r" (A), "r" (s), "r" (q), "r" (p), "r" (r)
  : "ax", "bx", "cx", "dx", "di", "si", "bp", "memory"
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
