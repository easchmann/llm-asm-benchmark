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
	"pushw %[nm]\n\t"
	"pushw %[nn]\n\t"
	"pushw %[pA]\n\t"
	"pushw %[pX]\n\t"
	"pushw %[pY]\n\t"
	"pushw %[pT]\n\t"
	"pushw %%bp\n\t"
	"movw %%sp, %%bp\n\t"
	"subw $4, %%sp\n\t"
	/* zero y */
	"movw -4(%%bp), %%cx\n\t"
	"jcxz 2f\n\t"
	"movw -10(%%bp), %%di\n\t"
	"xorw %%ax, %%ax\n\t"
	"1:\n\t"
	"movw %%ax, (%%di)\n\t"
	"addw $2, %%di\n\t"
	"loop 1b\n\t"
	"2:\n\t"
	/* init per-row state */
	"movw -6(%%bp), %%ax\n\t"
	"movw %%ax, -16(%%bp)\n\t"
	"movw $0, -14(%%bp)\n\t"
	"cmpw $0, -2(%%bp)\n\t"
	"jne 3f\n\t"
	"jmp 8f\n\t"
	"3:\n\t"
	/* tmp[i] = 0 */
	"movw -12(%%bp), %%ax\n\t"
	"movw -14(%%bp), %%dx\n\t"
	"addw %%dx, %%dx\n\t"
	"addw %%dx, %%ax\n\t"
	"movw %%ax, %%bx\n\t"
	"movw $0, (%%bx)\n\t"
	"movw %%bx, %%di\n\t"
	/* tmp[i] += A[i][j] * x[j] */
	"movw -16(%%bp), %%bx\n\t"
	"movw -8(%%bp), %%si\n\t"
	"movw -4(%%bp), %%cx\n\t"
	"jcxz 5f\n\t"
	"4:\n\t"
	"movw (%%bx), %%ax\n\t"
	"imulw (%%si)\n\t"
	"addw %%ax, (%%di)\n\t"
	"addw $2, %%bx\n\t"
	"addw $2, %%si\n\t"
	"loop 4b\n\t"
	"5:\n\t"
	/* y[j] += A[i][j] * tmp[i] */
	"movw -16(%%bp), %%bx\n\t"
	"movw -10(%%bp), %%si\n\t"
	"movw -4(%%bp), %%cx\n\t"
	"jcxz 7f\n\t"
	"6:\n\t"
	"movw (%%bx), %%ax\n\t"
	"imulw (%%si)\n\t"
	"addw %%ax, (%%si)\n\t"
	"addw $2, %%bx\n\t"
	"addw $2, %%si\n\t"
	"loop 6b\n\t"
	"7:\n\t"
	/* next row */
	"movw -4(%%bp), %%ax\n\t"
	"addw %%ax, %%ax\n\t"
	"addw %%ax, -16(%%bp)\n\t"
	"incw -14(%%bp)\n\t"
	"movw -14(%%bp), %%ax\n\t"
	"cmpw -2(%%bp), %%ax\n\t"
	"jl 3b\n\t"
	"8:\n\t"
	"movw %%bp, %%sp\n\t"
	"popw %%bp\n\t"
	: [nm] "+r" (m), [nn] "+r" (n), [pA] "+r" (A),
	  [pX] "+r" (x), [pY] "+r" (y), [pT] "+r" (tmp)
	:
	: "memory", "cc");
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
