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
    ".fpu 287\n\t"
    "movw %0, %%cx\n\t"
    "orw %%cx, %%cx\n\t"
    "jz 101f\n\t"
    "movw %2, %%si\n\t"
    "100:\n\t"
    "movw $0, (%%si)\n\t"
    "movw $0, 2(%%si)\n\t"
    "movw $0, 4(%%si)\n\t"
    "movw $0, 6(%%si)\n\t"
    "addw $8, %%si\n\t"
    "decw %%cx\n\t"
    "jnz 100b\n\t"
    "101:\n\t"
    "movw %1, %%cx\n\t"
    "orw %%cx, %%cx\n\t"
    "jz 201f\n\t"
    "movw %4, %%ax\n\t"
    "movw %6, %%di\n\t"
    "200:\n\t"
    "movw %1, %%dx\n\t"
    "subw %%cx, %%dx\n\t"
    "addw %%dx, %%dx\n\t"
    "addw %%dx, %%dx\n\t"
    "addw %%dx, %%dx\n\t"
    "movw %5, %%si\n\t"
    "addw %%dx, %%si\n\t"
    "fldl (%%si)\n\t"
    "movw %2, %%si\n\t"
    "movw %3, %%bx\n\t"
    "fldz\n\t"
    "movw %0, %%dx\n\t"
    "orw %%dx, %%dx\n\t"
    "jz 301f\n\t"
    "300:\n\t"
    "fldl (%%di)\n\t"
    "fmul %%st(2)\n\t"
    "fldl (%%si)\n\t"
    "fadd\n\t"
    "fstpl (%%si)\n\t"
    "fstp %%st\n\t"
    "fstp %%st\n\t"
    "fldl (%%di)\n\t"
    "fldl (%%bx)\n\t"
    "fmul %%st(1)\n\t"
    "faddp %%st(2)\n\t"
    "fstp %%st\n\t"
    "fstpl (%%ax)\n\t"
    "addw $8, %%si\n\t"
    "addw $8, %%bx\n\t"
    "addw $8, %%di\n\t"
    "decw %%dx\n\t"
    "jnz 300b\n\t"
    "301:\n\t"
    "fstpl (%%ax)\n\t"
    "fstp %%st\n\t"
    "fstp %%st\n\t"
    "movw %0, %%dx\n\t"
    "addw %%dx, %%dx\n\t"
    "addw %%dx, %%dx\n\t"
    "addw %%dx, %%dx\n\t"
    "addw %%dx, %%di\n\t"
    "addw $8, %%ax\n\t"
    "decw %%cx\n\t"
    "jnz 200b\n\t"
    "201:\n\t"
    "wait\n\t"
    :
    : "m"(m), "m"(n), "m"(s), "m"(p), "m"(q), "m"(r), "m"(A)
    : "ax", "bx", "cx", "dx", "si", "di", "cc", "memory"
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
