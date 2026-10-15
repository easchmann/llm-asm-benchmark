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
    "movw %2, %%cx\n\t"
    "movw %5, %%si\n\t"
    ".Latax_zero:\n\t"
    "cmpw $0, %%cx\n\t"
    "jz .Latax_zero_done\n\t"
    "movw $0, (%%si)\n\t"
    "addw $2, %%si\n\t"
    "decw %%cx\n\t"
    "jne .Latax_zero\n\t"
    ".Latax_zero_done:\n\t"
    "movw %1, %%cx\n\t"
    "movw %3, %%di\n\t"
    "movw %6, %%bx\n\t"
    ".Latax_outer:\n\t"
    "cmpw $0, %%cx\n\t"
    "jz .Latax_outer_done\n\t"
    "pushw %%cx\n\t"
    "pushw %%bx\n\t"
    "movw $0, %%bx\n\t"
    "movw %2, %%cx\n\t"
    "movw %4, %%si\n\t"
    ".Latax_dot:\n\t"
    "cmpw $0, %%cx\n\t"
    "jz .Latax_dot_done\n\t"
    "movw (%%si), %%ax\n\t"
    "imulw (%%di)\n\t"
    "addw %%ax, %%bx\n\t"
    "addw $2, %%di\n\t"
    "addw $2, %%si\n\t"
    "decw %%cx\n\t"
    "jne .Latax_dot\n\t"
    ".Latax_dot_done:\n\t"
    "movw %%bx, %%ax\n\t"
    "popw %%si\n\t"
    "movw %%ax, (%%si)\n\t"
    "addw $2, %%si\n\t"
    "popw %%cx\n\t"
    "movw %2, %%ax\n\t"
    "shl $1, %%ax\n\t"
    "subw %%ax, %%di\n\t"
    "pushw %%cx\n\t"
    "pushw %%si\n\t"
    "movw %5, %%si\n\t"
    "movw %2, %%cx\n\t"
    ".Latax_y:\n\t"
    "cmpw $0, %%cx\n\t"
    "jz .Latax_y_done\n\t"
    "movw (%%di), %%ax\n\t"
    "imulw %%bx\n\t"
    "addw %%ax, (%%si)\n\t"
    "addw $2, %%di\n\t"
    "addw $2, %%si\n\t"
    "decw %%cx\n\t"
    "jne .Latax_y\n\t"
    ".Latax_y_done:\n\t"
    "popw %%si\n\t"
    "movw %%si, %%bx\n\t"
    "popw %%cx\n\t"
    "movw %2, %%ax\n\t"
    "shl $1, %%ax\n\t"
    "addw %%ax, %%di\n\t"
    "decw %%cx\n\t"
    "jne .Latax_outer\n\t"
    ".Latax_outer_done:\n\t"
    :
    : "m" (m), "m" (n), "m" (A), "m" (x), "m" (y), "m" (tmp)
    : "ax", "bx", "cx", "dx", "si", "di", "memory", "cc"
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
