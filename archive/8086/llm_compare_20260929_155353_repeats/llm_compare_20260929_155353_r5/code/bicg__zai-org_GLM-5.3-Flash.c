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
    "	xorw %%ax, %%ax\n"
    "	movw %[m], %%cx\n"
    "	jcxz 4f\n"
    "	movw %[Sp], %%di\n"
    "1:\n"
    "	movw %%ax, (%%di)\n"
    "	addw $2, %%di\n"
    "	loop 1b\n"
    "4:\n"
    "	movw %[Rp], %%bx\n"
    "2:\n"
    "	movw %%bx, %%ax\n"
    "	subw %[Rp], %%ax\n"
    "	shrw $1, %%ax\n"
    "	cmpw %[n], %%ax\n"
    "	jae 3f\n"
    "	movw %[Ap], %%si\n"
    "	addw %%ax, %%ax\n"
    "	movw %[m], %%dx\n"
    "	mulw %%dx\n"
    "	addw %%ax, %%si\n"
    "	movw %[Sp], %%di\n"
    "	movw %[m], %%cx\n"
    "	jcxz 5f\n"
    "1:\n"
    "	movw (%%bx), %%ax\n"
    "	mulw (%%si)\n"
    "	addw %%ax, (%%di)\n"
    "	addw $2, %%si\n"
    "	addw $2, %%di\n"
    "	loop 1b\n"
    "5:\n"
    "	addw $2, %%bx\n"
    "	jmp 2b\n"
    "3:\n"
    "	movw %[Qp], %%bx\n"
    "	movw %[Pp], %%di\n"
    "6:\n"
    "	movw %%bx, %%ax\n"
    "	subw %[Qp], %%ax\n"
    "	shrw $1, %%ax\n"
    "	cmpw %[n], %%ax\n"
    "	jae 7f\n"
    "	movw %[Ap], %%si\n"
    "	addw %%ax, %%ax\n"
    "	movw %[m], %%dx\n"
    "	mulw %%dx\n"
    "	addw %%ax, %%si\n"
    "	movw %[Pp], %%di\n"
    "	movw $0, (%%bx)\n"
    "	movw %[m], %%cx\n"
    "	jcxz 8f\n"
    "1:\n"
    "	movw (%%si), %%ax\n"
    "	mulw (%%di)\n"
    "	addw %%ax, (%%bx)\n"
    "	addw $2, %%si\n"
    "	addw $2, %%di\n"
    "	loop 1b\n"
    "8:\n"
    "	addw $2, %%bx\n"
    "	jmp 6b\n"
    "7:\n"
    :
    : [m] "m" (m), [n] "m" (n),
      [Ap] "m" (A), [Sp] "m" (s), [Qp] "m" (q),
      [Pp] "m" (p), [Rp] "m" (r)
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
