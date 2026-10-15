/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* atax.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
#include "atax.h"


/* Array initialization. */
static
void init_array (int m, int n,
		 DATA_TYPE POLYBENCH_2D(A,M,N,m,n),
		 DATA_TYPE POLYBENCH_1D(x,N,n))
{
  int i, j;
  DATA_TYPE fn;
  fn = (DATA_TYPE)n;

  for (i = 0; i < n; i++)
      x[i] = 1 + (i / fn);
  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
      A[i][j] = (DATA_TYPE) ((i+j) % n) / (5*m);
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int n,
		 DATA_TYPE POLYBENCH_1D(y,N,n))

{
  int i;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("y");
  for (i = 0; i < n; i++) {
    if (i % 20 == 0) fprintf (POLYBENCH_DUMP_TARGET, "\n");
    fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, y[i]);
  }
  POLYBENCH_DUMP_END("y");
  POLYBENCH_DUMP_FINISH;
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_atax(int m, int n,
		 DATA_TYPE POLYBENCH_2D(A,M,N,m,n),
		 DATA_TYPE POLYBENCH_1D(x,N,n),
		 DATA_TYPE POLYBENCH_1D(y,N,n),
		 DATA_TYPE POLYBENCH_1D(tmp,M,m))
{
  int i, j;

#pragma scop
asm volatile (
    "cld\n\t"
    "movw %8, %%cx\n\t"
    "movw %4, %%di\n\t"
    "xorw %%ax, %%ax\n\t"
    "rep stosb\n\t"
    "xorw %%dx, %%dx\n\t"
    "1:\n\t"
    "cmpw %0, %%dx\n\t"
    "jae 2f\n\t"
    "movw %2, %%bx\n\t"
    "movw %5, %%ax\n\t"
    "imulw %%dx\n\t"
    "addw %%ax, %%bx\n\t"
    "movw %3, %%si\n\t"
    "xorw %%di, %%di\n\t"
    "xorw %%cx, %%cx\n\t"
    "3:\n\t"
    "cmpw %1, %%cx\n\t"
    "jae 4f\n\t"
    "movw (%%bx), %%ax\n\t"
    "imulw (%%si)\n\t"
    "addw %%ax, %%di\n\t"
    "addw $%6, %%bx\n\t"
    "addw $%6, %%si\n\t"
    "incw %%cx\n\t"
    "jmp 3b\n\t"
    "4:\n\t"
    "movw $%6, %%ax\n\t"
    "imulw %%dx\n\t"
    "movw %7, %%si\n\t"
    "addw %%ax, %%si\n\t"
    "movw %%di, (%%si)\n\t"
    "movw %2, %%bx\n\t"
    "movw %5, %%ax\n\t"
    "imulw %%dx\n\t"
    "addw %%ax, %%bx\n\t"
    "movw %4, %%si\n\t"
    "xorw %%cx, %%cx\n\t"
    "5:\n\t"
    "cmpw %1, %%cx\n\t"
    "jae 6f\n\t"
    "movw (%%bx), %%ax\n\t"
    "imulw %%di\n\t"
    "addw %%ax, (%%si)\n\t"
    "addw $%6, %%bx\n\t"
    "addw $%6, %%si\n\t"
    "incw %%cx\n\t"
    "jmp 5b\n\t"
    "6:\n\t"
    "incw %%dx\n\t"
    "jmp 1b\n\t"
    "2:\n\t"
    :
    : "m"(m), "m"(n), "m"(A), "m"(x), "m"(y),
      "m"(_row_inc), "i"((int)sizeof(DATA_TYPE)), "m"(tmp), "m"(_y_bytes)
    : "ax", "bx", "cx", "dx", "si", "di", "memory"
  );
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int m = M;
  int n = N;

  /* Variable declaration/allocation. */
  POLYBENCH_2D_ARRAY_DECL(A, DATA_TYPE, M, N, m, n);
  POLYBENCH_1D_ARRAY_DECL(x, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(y, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(tmp, DATA_TYPE, M, m);

  /* Initialize array(s). */
  init_array (m, n, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(x));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_atax (m, n,
	       POLYBENCH_ARRAY(A),
	       POLYBENCH_ARRAY(x),
	       POLYBENCH_ARRAY(y),
	       POLYBENCH_ARRAY(tmp));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(n, POLYBENCH_ARRAY(y)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(x);
  POLYBENCH_FREE_ARRAY(y);
  POLYBENCH_FREE_ARRAY(tmp);

  return 0;
}
