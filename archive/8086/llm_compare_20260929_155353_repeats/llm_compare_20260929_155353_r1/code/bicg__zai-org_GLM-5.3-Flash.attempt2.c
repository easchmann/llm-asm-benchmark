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
    "movw %7, %%cx\n\t"
    "movw %5, %%di\n\t"
    "jcxz 7f\n"
    "0:\n\t"
    "movw $0, (%%di)\n\t"
    "addw $2, %%di\n\t"
    "loop 0b\n"
    "7:\n\t"
    "movw %8, %%ax\n\t"
    "movw %%ax, %2\n\t"
    "movw %4, %%bx\n\t"
    "jmp 6f\n"
    "1:\n\t"
    "movw %0, %%di\n\t"
    "movw (%%di), %%dx\n\t"
    "movw %%dx, %3\n\t"
    "addw $2, %0\n\t"
    "xorw %%si, %%si\n\t"
    "xorw %%cx, %%cx\n"
    "2:\n\t"
    "movw (%%bx,%%si), %%ax\n\t"
    "imulw %3\n\t"
    "movw %5, %%di\n\t"
    "addw %%si, %%di\n\t"
    "addw %%ax, (%%di)\n\t"
    "movw (%%bx,%%si), %%ax\n\t"
    "movw %6, %%di\n\t"
    "addw %%si, %%di\n\t"
    "imulw (%%di)\n\t"
    "addw %%ax, %%cx\n\t"
    "addw $2, %%si\n\t"
    "movw %7, %%ax\n\t"
    "addw %%ax, %%ax\n\t"
    "cmpw %%ax, %%si\n\t"
    "jge 8f\n\t"
    "jmp 2b\n"
    "8:\n\t"
    "movw %1, %%di\n\t"
    "movw %%cx, (%%di)\n\t"
    "addw $2, %1\n\t"
    "movw %7, %%ax\n\t"
    "addw %%ax, %%ax\n\t"
    "addw %%ax, %%bx\n\t"
    "decw %2\n"
    "6:\n\t"
    "cmpw $0, %2\n\t"
    "je 4f\n\t"
    "jmp 1b\n"
    "4:"
    : [rP] "+m" (r),
      [qP] "+m" (q),
      [ic] "+m" (ic),
      [rv] "+m" (rv)
    : [Ap] "m" (A),
      [Sp] "m" (s),
      [Pp] "m" (p),
      [mm] "m" (m),
      [nn] "m" (n)
    : "ax", "bx", "cx", "dx", "si", "di", "cc", "memory");
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
