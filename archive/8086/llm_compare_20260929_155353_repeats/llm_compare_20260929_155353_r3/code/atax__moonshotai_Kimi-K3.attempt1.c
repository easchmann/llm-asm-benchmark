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
  "movw %[y_ptr], %%di\n\t"
  "movw %[nval], %%cx\n\t"
  "jcxz 1f\n\t"
  "xorw %%ax, %%ax\n"
  "0:\n\t"
  "movw %%ax, (%%di)\n\t"
  "movw %%ax, 2(%%di)\n\t"
  "movw %%ax, 4(%%di)\n\t"
  "movw %%ax, 6(%%di)\n\t"
  "addw $8, %%di\n\t"
  "loop 0b\n"
  "1:\n\t"
  "movw %[mval], %%dx\n\t"
  "testw %%dx, %%dx\n\t"
  "jz 9f\n\t"
  "movw %[A_ptr], %%si\n\t"
  "movw %[tmp_ptr], %%bx\n"
  "2:\n\t"
  "xorw %%ax, %%ax\n\t"
  "movw %%ax, (%%bx)\n\t"
  "movw %%ax, 2(%%bx)\n\t"
  "movw %%ax, 4(%%bx)\n\t"
  "movw %%ax, 6(%%bx)\n\t"
  "movw %[x_ptr], %%di\n\t"
  "movw %[nval], %%cx\n\t"
  "push %%si\n\t"
  "jcxz 4f\n"
  "3:\n\t"
  "fldl (%%si)\n\t"
  "fmull (%%di)\n\t"
  "faddl (%%bx)\n\t"
  "fstpl (%%bx)\n\t"
  "addw $8, %%si\n\t"
  "addw $8, %%di\n\t"
  "loop 3b\n"
  "4:\n\t"
  "pop %%si\n\t"
  "movw %[y_ptr], %%di\n\t"
  "movw %[nval], %%cx\n\t"
  "jcxz 6f\n"
  "5:\n\t"
  "fldl (%%si)\n\t"
  "fmull (%%bx)\n\t"
  "faddl (%%di)\n\t"
  "fstpl (%%di)\n\t"
  "addw $8, %%si\n\t"
  "addw $8, %%di\n\t"
  "loop 5b\n"
  "6:\n\t"
  "addw $8, %%bx\n\t"
  "decw %%dx\n\t"
  "jnz 2b\n"
  "9:"
  :
  : [A_ptr] "m" (A), [x_ptr] "m" (x), [y_ptr] "m" (y), [tmp_ptr] "m" (tmp),
    [mval] "m" (_PB_M), [nval] "m" (_PB_N)
  : "ax", "bx", "cx", "dx", "si", "di", "cc", "memory");
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
