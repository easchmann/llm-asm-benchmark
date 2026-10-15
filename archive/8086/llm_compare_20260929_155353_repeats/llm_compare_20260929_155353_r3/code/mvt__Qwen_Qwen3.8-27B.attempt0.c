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
    "movw %x1p, %%si\n\t"
    "movw %x2p, %%di\n\t"
    "movw %y1p, %%bx\n\t"
    "movw %y2p, %%cx\n\t"
    "movw %Ap, %%ax\n\t"
    "movw %np, %%dx\n\t"
    "pushw %%bp\n\t"
    "subw $20, %%sp\n\t"
    "movw %%sp, %%bp\n\t"
    "movw %%si, (%%bp)\n\t"
    "movw %%di, 2(%%bp)\n\t"
    "movw %%bx, 4(%%bp)\n\t"
    "movw %%cx, 6(%%bp)\n\t"
    "movw %%ax, 8(%%bp)\n\t"
    "movw %%dx, 10(%%bp)\n\t"
    "movw 10(%%bp), %%ax\n\t"
    "jz .Lmvt_scop_done\n\t"
    "movw 10(%%bp), %%ax\n\t"
    "movw %elem, %%bx\n\t"
    "mulw %%bx\n\t"
    "movw %%ax, 12(%%bp)\n\t"
    "movw 12(%%bp), %%ax\n\t"
    "movw 10(%%bp), %%bx\n\t"
    "mulw %%bx\n\t"
    "movw %%ax, 14(%%bp)\n\t"
    "xorw %%cx, %%cx\n\t"
    "movw %%cx, 16(%%bp)\n\t"
    "movw (%%bp), %%si\n\t"
    "movw 8(%%bp), %%di\n\t"
    ".Lmvt_scop_i1:\n\t"
    "movw 4(%%bp), %%bx\n\t"
    "xorw %%cx, %%cx\n\t"
    ".Lmvt_scop_j1:\n\t"
    "movw (%%di), %%ax\n\t"
    "mulw (%%bx)\n\t"
    "addw %%ax, (%%si)\n\t"
    "addw %elem, %%di\n\t"
    "addw %elem, %%bx\n\t"
    "incw %%cx\n\t"
    "cmpw 10(%%bp), %%cx\n\t"
    "jl .Lmvt_scop_j1\n\t"
    "addw %elem, %%si\n\t"
    "incw 16(%%bp)\n\t"
    "movw 16(%%bp), %%cx\n\t"
    "movw 10(%%bp), %%dx\n\t"
    "cmpw %%dx, %%cx\n\t"
    "jl .Lmvt_scop_i1\n\t"
    "xorw %%cx, %%cx\n\t"
    "movw %%cx, 16(%%bp)\n\t"
    "movw 2(%%bp), %%si\n\t"
    "movw 8(%%bp), %%di\n\t"
    ".Lmvt_scop_i2:\n\t"
    "movw 6(%%bp), %%bx\n\t"
    "xorw %%cx, %%cx\n\t"
    ".Lmvt_scop_j2:\n\t"
    "movw (%%di), %%ax\n\t"
    "mulw (%%bx)\n\t"
    "addw %%ax, (%%si)\n\t"
    "addw %elem, %%bx\n\t"
    "movw 12(%%bp), %%dx\n\t"
    "addw %%dx, %%di\n\t"
    "incw %%cx\n\t"
    "cmpw 10(%%bp), %%cx\n\t"
    "jl .Lmvt_scop_j2\n\t"
    "addw %elem, %%si\n\t"
    "movw 14(%%bp), %%dx\n\t"
    "subw %%dx, %%di\n\t"
    "addw %elem, %%di\n\t"
    "incw 16(%%bp)\n\t"
    "movw 16(%%bp), %%cx\n\t"
    "movw 10(%%bp), %%dx\n\t"
    "cmpw %%dx, %%cx\n\t"
    "jl .Lmvt_scop_i2\n\t"
    ".Lmvt_scop_done:\n\t"
    "addw $20, %%sp\n\t"
    "popw %%bp\n\t"
    :
    : [x1p] "m" (x1),
      [x2p] "m" (x2),
      [y1p] "m" (y_1),
      [y2p] "m" (y_2),
      [Ap] "m" (A),
      [np] "m" (_asm_bound),
      [elem] "i" (sizeof(DATA_TYPE))
    : "memory", "ax", "bx", "cx", "dx", "si", "di", "flags"
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
