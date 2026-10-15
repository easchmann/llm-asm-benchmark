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
    "movw %0, %%ax\n\t"
    "movw %5, %%dx\n\t"
    "movw %2, %%si\n\t"
    "movw %3, %%di\n\t"
    "movw %4, %%bx\n\t"
    "movw %1, %%cx\n\t"
    "pushw %%bp\n\t"
    "movw %%sp, %%bp\n\t"
    "subw $16, %%sp\n\t"
    "movw %%ax, -2(%%bp)\n\t"
    "addw %%ax, %%ax\n\t"
    "addw %%ax, %%ax\n\t"
    "movw %%ax, -4(%%bp)\n\t"
    "movw %%di, -10(%%bp)\n\t"
    "movw %%bx, -12(%%bp)\n\t"
    "movw %%si, -14(%%bp)\n\t"
    "movw %%cx, -16(%%bp)\n\t"
    "movw -2(%%bp), %%cx\n\t"
    "jcxz 1f\n\t"
    "0:\n\t"
    "fldz\n\t"
    "fstps (%%bx)\n\t"
    "addw $4, %%bx\n\t"
    "loop 0b\n\t"
    "1:\n\t"
    "movw -16(%%bp), %%ax\n\t"
    "orw %%ax, %%ax\n\t"
    "jnz 9f\n\t"
    "jmp 8f\n\t"
    "9:\n\t"
    "fldz\n\t"
    "fstps (%%dx)\n\t"
    "movw -10(%%bp), %%di\n\t"
    "movw -14(%%bp), %%si\n\t"
    "movw -2(%%bp), %%cx\n\t"
    "jcxz 2f\n\t"
    "0:\n\t"
    "flds (%%si)\n\t"
    "fmuls (%%di)\n\t"
    "fadds (%%dx)\n\t"
    "fstps (%%dx)\n\t"
    "addw $4, %%si\n\t"
    "addw $4, %%di\n\t"
    "loop 0b\n\t"
    "2:\n\t"
    "movw -14(%%bp), %%si\n\t"
    "movw -12(%%bp), %%bx\n\t"
    "movw -2(%%bp), %%cx\n\t"
    "jcxz 3f\n\t"
    "0:\n\t"
    "flds (%%si)\n\t"
    "fmuls (%%dx)\n\t"
    "fadds (%%bx)\n\t"
    "fstps (%%bx)\n\t"
    "addw $4, %%si\n\t"
    "addw $4, %%bx\n\t"
    "loop 0b\n\t"
    "3:\n\t"
    "movw -4(%%bp), %%ax\n\t"
    "addw %%ax, -14(%%bp)\n\t"
    "addw $4, %%dx\n\t"
    "movw -16(%%bp), %%ax\n\t"
    "decw %%ax\n\t"
    "movw %%ax, -16(%%bp)\n\t"
    "jmp 1b\n\t"
    "8:\n\t"
    "movw %%bp, %%sp\n\t"
    "popw %%bp\n\t"
    : : "m"(n), "m"(m), "m"(A), "m"(x), "m"(y), "m"(tmp)
    : "ax","bx","cx","dx","si","di","cc","memory");
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
