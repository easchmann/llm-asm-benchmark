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
    // Load fixed base pointers
    "movw %[A], %bx\n\t"
    "movw %[x], %si\n\t"
    "movw %[y], %di\n\t"
    "movw %[tmp], %bp\n\t"

    // Initialize y to 0
    "movw $0, %ax\n\t"
    "movw $%[N], %cx\n\t"
    "y_init_loop:\n\t"
    "movq $%[zero], (%di, %ax)\n\t"  // assuming SCALAR_VAL(0.0) is %[zero]
    "addw $%[S], %ax\n\t"
    "loop y_init_loop\n\t"

    // i loop
    "movw $0, %dx\n\t"
    "movw $%[M], %cx\n\t"
    "i_loop:\n\t"

    // Compute row_base = A_base + i*N*S
    "pushw %si\n\t"
    "movw $%[row_stride], %si\n\t"  // row_stride = N*S
    "movw %dx, %ax\n\t"
    "mulw %si\n\t"
    "popw %si\n\t"
    "addw %ax, %bx\n\t"  // bx = row_base

    // Compute tmp[i] address and initialize to 0
    "movw %dx, %ax\n\t"
    "movw $%[S], %si\n\t"
    "mulw %si\n\t"
    "addw %ax, %bp\n\t"  // bp = tmp[i]
    "movq $%[zero], (%bp)\n\t"

    // j1 loop: tmp[i] += sum(A[i][j] * x[j])
    "fld (%bp)\n\t"
    "movw $0, %ax\n\t"
    "movw $%[N], %cx\n\t"
    "j1_loop:\n\t"
    "movq (%bx, %ax), %st(1)\n\t"
    "movq (%si, %ax), %st(2)\n\t"
    "fmul %st(2), %st(1)\n\t"
    "fadd %st(1), %st(0)\n\t"
    "addw $%[S], %ax\n\t"
    "loop j1_loop\n\t"
    "fstp (%bp)\n\t"

    // j2 loop: y[j] += A[i][j] * tmp[i]
    "fld (%bp)\n\t"
    "movw $0, %ax\n\t"
    "movw $%[N], %cx\n\t"
    "j2_loop:\n\t"
    "movq (%bx, %ax), %st(1)\n\t"
    "fmul %st(0), %st(1)\n\t"
    "fld (%di, %ax)\n\t"
    "fadd %st(1), %st(0)\n\t"
    "fstp (%di, %ax)\n\t"
    "addw $%[S], %ax\n\t"
    "loop j2_loop\n\t"

    // Next i
    "incw %dx\n\t"
    "loop i_loop\n\t"

    :  // no output operands (all modifications are to memory)
    : [A] "r" (A), [x] "r" (x), [y] "r" (y), [tmp] "r" (tmp),
      [M] "i" (_PB_M), [N] "i" (_PB_N), [S] "i" (sizeof(DATA_TYPE)),
      [row_stride] "i" (_PB_N * sizeof(DATA_TYPE)),
      [zero] "X" (SCALAR_VAL(0.0))  // X constraint for FPU constant
    : "ax", "bx", "cx", "dx", "si", "di", "bp", "memory", "st"  // clobbered registers
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
