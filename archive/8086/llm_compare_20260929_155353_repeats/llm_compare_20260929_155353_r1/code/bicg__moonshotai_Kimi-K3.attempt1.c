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
  "movw %[mb], %%cx\n\t"
  "movw %[S], %%bx\n\t"
  "testw %%cx, %%cx\n\t"
  "jz 2f\n"
  "1:\n\t"
  "movw $0, (%%bx)\n\t"
  "movw $0, 2(%%bx)\n\t"
  "movw $0, 4(%%bx)\n\t"
  "movw $0, 6(%%bx)\n\t"
  "addw $8, %%bx\n\t"
  "decw %%cx\n\t"
  "jnz 1b\n"
  "2:\n\t"
  "movw %[nb], %%ax\n\t"
  "movw %[A], %%bx\n\t"
  "movw %[R], %%dx\n\t"
  "pushw %%bp\n\t"
  "pushw %[mb]\n\t"
  "pushw %[P]\n\t"
  "pushw %[S]\n\t"
  "movw %[Q], %%bp\n\t"
  "testw %%ax, %%ax\n\t"
  "jz 6f\n"
  "3:\n\t"
  "movw %%dx, %%si\n\t"
  "fldz\n\t"
  "fstpl %%ds:(%%bp)\n\t"
  "fldl (%%si)\n\t"
  "popw %%si\n\t"
  "popw %%di\n\t"
  "popw %%cx\n\t"
  "pushw %%cx\n\t"
  "pushw %%di\n\t"
  "pushw %%si\n\t"
  "testw %%cx, %%cx\n\t"
  "jz 5f\n"
  "4:\n\t"
  "fld %%st(0)\n\t"
  "fmull (%%bx)\n\t"
  "faddl (%%si)\n\t"
  "fstpl (%%si)\n\t"
  "fldl (%%bx)\n\t"
  "fmull (%%di)\n\t"
  "faddl %%ds:(%%bp)\n\t"
  "fstpl %%ds:(%%bp)\n\t"
  "addw $8, %%bx\n\t"
  "addw $8, %%si\n\t"
  "addw $8, %%di\n\t"
  "decw %%cx\n\t"
  "jnz 4b\n"
  "5:\n\t"
  "fstp %%st(0)\n\t"
  "addw $8, %%bp\n\t"
  "addw $8, %%dx\n\t"
  "decw %%ax\n\t"
  "jnz 3b\n"
  "6:\n\t"
  "popw %%si\n\t"
  "popw %%di\n\t"
  "popw %%cx\n\t"
  "popw %%bp\n\t"
  :
  : [mb] "m" (m), [nb] "m" (n),
    [A] "m" (A), [S] "m" (s), [Q] "m" (q),
    [P] "m" (p), [R] "m" (r)
  : "cc", "memory", "ax", "bx", "cx", "dx", "si", "di");
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
