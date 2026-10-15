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
asm(
    "subw $32, %%sp\n\t"
    "movw $0, (%%sp)\n\t"
    "movw $0, 2(%%sp)\n\t"
    ".L1:\n\t"
    "movw (%%sp), %%ax\n\t"
    "movw 8(%%bp), %%cx\n\t"
    "cmpw %%cx, %%ax\n\t"
    "jge .L2\n\t"
    "movw (%%sp), %%si\n\t"
    "shl $3, %%si\n\t"
    "movw 16(%%bp), %%di\n\t"
    "addw %%si, %%di\n\t"
    "movw $0, (%%di)\n\t"
    "movw $0, 2(%%di)\n\t"
    "movw $0, 4(%%di)\n\t"
    "movw $0, 6(%%di)\n\t"
    "incw (%%sp)\n\t"
    "jnz .L1a\n\t"
    "incw 2(%%sp)\n\t"
    ".L1a:\n\t"
    "jmp .L1\n\t"
    ".L2:\n\t"
    "movw $0, (%%sp)\n\t"
    "movw $0, 2(%%sp)\n\t"
    ".L3:\n\t"
    "movw (%%sp), %%ax\n\t"
    "movw 4(%%bp), %%cx\n\t"
    "cmpw %%cx, %%ax\n\t"
    "jge .L4\n\t"
    "movw (%%sp), %%si\n\t"
    "shl $3, %%si\n\t"
    "movw 18(%%bp), %%di\n\t"
    "addw %%si, %%di\n\t"
    "movw $0, (%%di)\n\t"
    "movw $0, 2(%%di)\n\t"
    "movw $0, 4(%%di)\n\t"
    "movw $0, 6(%%di)\n\t"
    "movw $0, 4(%%sp)\n\t"
    "movw $0, 6(%%sp)\n\t"
    ".L5:\n\t"
    "movw 4(%%sp), %%ax\n\t"
    "movw 8(%%bp), %%cx\n\t"
    "cmpw %%cx, %%ax\n\t"
    "jge .L6\n\t"
    "movw (%%sp), %%ax\n\t"
    "movw 8(%%bp), %%cx\n\t"
    "mulw %%cx\n\t"
    "movw %%ax, %%bx\n\t"
    "movw 4(%%sp), %%cx\n\t"
    "addw %%cx, %%bx\n\t"
    "shl $3, %%bx\n\t"
    "movw 12(%%bp), %%di\n\t"
    "addw %%bx, %%di\n\t"
    "movw %%di, 16(%%sp)\n\t"
    "movw 4(%%sp), %%si\n\t"
    "shl $3, %%si\n\t"
    "movw 14(%%bp), %%di\n\t"
    "addw %%si, %%di\n\t"
    "movw %%di, 18(%%sp)\n\t"
    "movw %%sp, %%si\n\t"
    "addw $8, %%si\n\t"
    "pushw 18(%%sp)\n\t"
    "pushw 16(%%sp)\n\t"
    "pushw %%si\n\t"
    "call __dmul\n\t"
    "addw $6, %%sp\n\t"
    "movw (%%sp), %%si\n\t"
    "shl $3, %%si\n\t"
    "movw 18(%%bp), %%di\n\t"
    "addw %%si, %%di\n\t"
    "movw %%di, 20(%%sp)\n\t"
    "movw %%sp, %%si\n\t"
    "addw $8, %%si\n\t"
    "pushw %%si\n\t"
    "pushw 20(%%sp)\n\t"
    "pushw 20(%%sp)\n\t"
    "call __dadd\n\t"
    "addw $6, %%sp\n\t"
    "incw 4(%%sp)\n\t"
    "jnz .L5a\n\t"
    "incw 6(%%sp)\n\t"
    ".L5a:\n\t"
    "jmp .L5\n\t"
    ".L6:\n\t"
    "movw $0, 4(%%sp)\n\t"
    "movw $0, 6(%%sp)\n\t"
    ".L7:\n\t"
    "movw 4(%%sp), %%ax\n\t"
    "movw 8(%%bp), %%cx\n\t"
    "cmpw %%cx, %%ax\n\t"
    "jge .L8\n\t"
    "movw (%%sp), %%ax\n\t"
    "movw 8(%%bp), %%cx\n\t"
    "mulw %%cx\n\t"
    "movw %%ax, %%bx\n\t"
    "movw 4(%%sp), %%cx\n\t"
    "addw %%cx, %%bx\n\t"
    "shl $3, %%bx\n\t"
    "movw 12(%%bp), %%di\n\t"
    "addw %%bx, %%di\n\t"
    "movw %%di, 16(%%sp)\n\t"
    "movw 4(%%sp), %%si\n\t"
    "shl $3, %%si\n\t"
    "movw 16(%%bp), %%di\n\t"
    "addw %%si, %%di\n\t"
    "movw %%di, 22(%%sp)\n\t"
    "movw %%sp, %%si\n\t"
    "addw $8, %%si\n\t"
    "pushw 20(%%sp)\n\t"
    "pushw 16(%%sp)\n\t"
    "pushw %%si\n\t"
    "call __dmul\n\t"
    "addw $6, %%sp\n\t"
    "movw %%sp, %%si\n\t"
    "addw $8, %%si\n\t"
    "pushw %%si\n\t"
    "pushw 22(%%sp)\n\t"
    "pushw 22(%%sp)\n\t"
    "call __dadd\n\t"
    "addw $6, %%sp\n\t"
    "incw 4(%%sp)\n\t"
    "jnz .L7a\n\t"
    "incw 6(%%sp)\n\t"
    ".L7a:\n\t"
    "jmp .L7\n\t"
    ".L8:\n\t"
    "incw (%%sp)\n\t"
    "jnz .L3a\n\t"
    "incw 2(%%sp)\n\t"
    ".L3a:\n\t"
    "jmp .L3\n\t"
    ".L4:\n\t"
    "addw $32, %%sp\n\t"
    :
    :
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
