/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* mvt.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
#include "mvt.h"


/* Array initialization. */
static
void init_array(int n,
		DATA_TYPE POLYBENCH_1D(x1,N,n),
		DATA_TYPE POLYBENCH_1D(x2,N,n),
		DATA_TYPE POLYBENCH_1D(y_1,N,n),
		DATA_TYPE POLYBENCH_1D(y_2,N,n),
		DATA_TYPE POLYBENCH_2D(A,N,N,n,n))
{
  int i, j;

  for (i = 0; i < n; i++)
    {
      x1[i] = (DATA_TYPE) (i % n) / n;
      x2[i] = (DATA_TYPE) ((i + 1) % n) / n;
      y_1[i] = (DATA_TYPE) ((i + 3) % n) / n;
      y_2[i] = (DATA_TYPE) ((i + 4) % n) / n;
      for (j = 0; j < n; j++)
	A[i][j] = (DATA_TYPE) (i*j % n) / n;
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int n,
		 DATA_TYPE POLYBENCH_1D(x1,N,n),
		 DATA_TYPE POLYBENCH_1D(x2,N,n))

{
  int i;

  POLYBENCH_DUMP_START;
  POLYBENCH_DUMP_BEGIN("x1");
  for (i = 0; i < n; i++) {
    if (i % 20 == 0) fprintf (POLYBENCH_DUMP_TARGET, "\n");
    fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, x1[i]);
  }
  POLYBENCH_DUMP_END("x1");

  POLYBENCH_DUMP_BEGIN("x2");
  for (i = 0; i < n; i++) {
    if (i % 20 == 0) fprintf (POLYBENCH_DUMP_TARGET, "\n");
    fprintf (POLYBENCH_DUMP_TARGET, DATA_PRINTF_MODIFIER, x2[i]);
  }
  POLYBENCH_DUMP_END("x2");
  POLYBENCH_DUMP_FINISH;
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_mvt(int n,
		DATA_TYPE POLYBENCH_1D(x1,N,n),
		DATA_TYPE POLYBENCH_1D(x2,N,n),
		DATA_TYPE POLYBENCH_1D(y_1,N,n),
		DATA_TYPE POLYBENCH_1D(y_2,N,n),
		DATA_TYPE POLYBENCH_2D(A,N,N,n,n))
{
  int i, j;

#pragma scop
__asm__ __volatile__ (
    "movw %[n], %%ax\n\t"
    "testw %%ax, %%ax\n\t"
    "jle .Lmvt_done%=\n\t"
    "movw %%ax, %%cx\n\t"
    "movw %[x1], %%di\n\t"
    "movw %[y1], %%si\n\t"
    "movw %[A], %%bx\n\t"
    ".Lmvt_outer1%=:\n\t"
    "movw %%ax, %%dx\n\t"
    "movw %[y1], %%si\n\t"
    ".Lmvt_inner1%=:\n\t"
    "fldl (%%bx)\n\t"
    "fmull (%%si)\n\t"
    "faddl (%%di)\n\t"
    "fstpl (%%di)\n\t"
    "addw $8, %%bx\n\t"
    "addw $8, %%si\n\t"
    "decw %%dx\n\t"
    "jnz .Lmvt_inner1%=\n\t"
    "addw $8, %%di\n\t"
    "decw %%cx\n\t"
    "jnz .Lmvt_outer1%=\n\t"
    "movw %[x2], %%di\n\t"
    "movw %[n], %%cx\n\t"
    ".Lmvt_outer2%=:\n\t"
    "movw %[n], %%dx\n\t"
    "movw %%dx, %%ax\n\t"
    "subw %%cx, %%ax\n\t"
    "addw %%ax, %%ax\n\t"
    "addw %%ax, %%ax\n\t"
    "addw %%ax, %%ax\n\t"
    "movw %[A], %%bx\n\t"
    "addw %%ax, %%bx\n\t"
    "movw %[y2], %%si\n\t"
    "movw %[n], %%ax\n\t"
    "addw %%ax, %%ax\n\t"
    "addw %%ax, %%ax\n\t"
    "addw %%ax, %%ax\n\t"
    ".Lmvt_inner2%=:\n\t"
    "fldl (%%bx)\n\t"
    "fmull (%%si)\n\t"
    "faddl (%%di)\n\t"
    "fstpl (%%di)\n\t"
    "addw %%ax, %%bx\n\t"
    "addw $8, %%si\n\t"
    "decw %%dx\n\t"
    "jnz .Lmvt_inner2%=\n\t"
    "addw $8, %%di\n\t"
    "decw %%cx\n\t"
    "jnz .Lmvt_outer2%=\n\t"
    ".Lmvt_done%=:"
    :
    : [n] "m" (mvt_asm_n),
      [x1] "m" (mvt_asm_x1),
      [x2] "m" (mvt_asm_x2),
      [y1] "m" (mvt_asm_y1),
      [y2] "m" (mvt_asm_y2),
      [A] "m" (mvt_asm_A)
    : "ax", "bx", "cx", "dx", "si", "di", "memory", "cc"
  );
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int n = N;

  /* Variable declaration/allocation. */
  POLYBENCH_2D_ARRAY_DECL(A, DATA_TYPE, N, N, n, n);
  POLYBENCH_1D_ARRAY_DECL(x1, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(x2, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(y_1, DATA_TYPE, N, n);
  POLYBENCH_1D_ARRAY_DECL(y_2, DATA_TYPE, N, n);


  /* Initialize array(s). */
  init_array (n,
	      POLYBENCH_ARRAY(x1),
	      POLYBENCH_ARRAY(x2),
	      POLYBENCH_ARRAY(y_1),
	      POLYBENCH_ARRAY(y_2),
	      POLYBENCH_ARRAY(A));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_mvt (n,
	      POLYBENCH_ARRAY(x1),
	      POLYBENCH_ARRAY(x2),
	      POLYBENCH_ARRAY(y_1),
	      POLYBENCH_ARRAY(y_2),
	      POLYBENCH_ARRAY(A));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(n, POLYBENCH_ARRAY(x1), POLYBENCH_ARRAY(x2)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(x1);
  POLYBENCH_FREE_ARRAY(x2);
  POLYBENCH_FREE_ARRAY(y_1);
  POLYBENCH_FREE_ARRAY(y_2);

  return 0;
}
