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
asm volatile (
  "pushw %[Rb]\n\t"
  "movw %[mb], %%ax\n\t"
  "movw %[nb], %%bx\n\t"
  "movw %[Ab], %%cx\n\t"
  "movw %[Sb], %%dx\n\t"
  "movw %[Qb], %%si\n\t"
  "movw %[Pb], %%di\n\t"
  "pushw %%bp\n\t"
  "movw %%sp, %%bp\n\t"
  "subw $28, %%sp\n\t"
  "movw %%bx, -2(%%bp)\n\t"
  "movw %%ax, -28(%%bp)\n\t"
  "movw %%cx, -14(%%bp)\n\t"
  "movw %%dx, -22(%%bp)\n\t"
  "movw %%si, -20(%%bp)\n\t"
  "movw %%di, -24(%%bp)\n\t"
  "movw 2(%%bp), %%ax\n\t"
  "movw %%ax, -26(%%bp)\n\t"
  "movw -28(%%bp), %%cx\n\t"
  "movw -22(%%bp), %%di\n\t"
  "testw %%cx, %%cx\n\t"
  "jz 2f\n"
  "1:\n\t"
  "movw $0, (%%di)\n\t"
  "movw $0, 2(%%di)\n\t"
  "movw $0, 4(%%di)\n\t"
  "movw $0, 6(%%di)\n\t"
  "addw $8, %%di\n\t"
  "decw %%cx\n\t"
  "jnz 1b\n"
  "2:\n\t"
  "cmpw $0, -2(%%bp)\n\t"
  "jne 3f\n\t"
  "jmp 20f\n"
  "3:\n\t"
  "movw -28(%%bp), %%ax\n\t"
  "movw %%ax, -4(%%bp)\n\t"
  "movw -22(%%bp), %%ax\n\t"
  "movw %%ax, -16(%%bp)\n\t"
  "movw -24(%%bp), %%ax\n\t"
  "movw %%ax, -18(%%bp)\n\t"
  "movw -20(%%bp), %%di\n\t"
  "movw $0, (%%di)\n\t"
  "movw $0, 2(%%di)\n\t"
  "movw $0, 4(%%di)\n\t"
  "movw $0, 6(%%di)\n\t"
  "movw -26(%%bp), %%si\n\t"
  "movw (%%si), %%ax\n\t"
  "movw %%ax, -12(%%bp)\n\t"
  "movw 2(%%si), %%ax\n\t"
  "movw %%ax, -10(%%bp)\n\t"
  "movw 4(%%si), %%ax\n\t"
  "movw %%ax, -8(%%bp)\n\t"
  "movw 6(%%si), %%ax\n\t"
  "movw %%ax, -6(%%bp)\n\t"
  "cmpw $0, -4(%%bp)\n\t"
  "jne 4f\n\t"
  "jmp 10f\n"
  "4:\n\t"
  "movw -14(%%bp), %%si\n\t"
  "pushw 6(%%si)\n\t"
  "pushw 4(%%si)\n\t"
  "pushw 2(%%si)\n\t"
  "pushw (%%si)\n\t"
  "pushw -6(%%bp)\n\t"
  "pushw -8(%%bp)\n\t"
  "pushw -10(%%bp)\n\t"
  "pushw -12(%%bp)\n\t"
  "call __muldf3\n\t"
  "addw $16, %%sp\n\t"
  "pushw %%bx\n\t"
  "pushw %%cx\n\t"
  "pushw %%dx\n\t"
  "pushw %%ax\n\t"
  "movw -16(%%bp), %%di\n\t"
  "pushw 6(%%di)\n\t"
  "pushw 4(%%di)\n\t"
  "pushw 2(%%di)\n\t"
  "pushw (%%di)\n\t"
  "call __adddf3\n\t"
  "addw $16, %%sp\n\t"
  "movw -16(%%bp), %%di\n\t"
  "movw %%ax, (%%di)\n\t"
  "movw %%dx, 2(%%di)\n\t"
  "movw %%cx, 4(%%di)\n\t"
  "movw %%bx, 6(%%di)\n\t"
  "movw -18(%%bp), %%di\n\t"
  "pushw 6(%%di)\n\t"
  "pushw 4(%%di)\n\t"
  "pushw 2(%%di)\n\t"
  "pushw (%%di)\n\t"
  "movw -14(%%bp), %%si\n\t"
  "pushw 6(%%si)\n\t"
  "pushw 4(%%si)\n\t"
  "pushw 2(%%si)\n\t"
  "pushw (%%si)\n\t"
  "call __muldf3\n\t"
  "addw $16, %%sp\n\t"
  "pushw %%bx\n\t"
  "pushw %%cx\n\t"
  "pushw %%dx\n\t"
  "pushw %%ax\n\t"
  "movw -20(%%bp), %%di\n\t"
  "pushw 6(%%di)\n\t"
  "pushw 4(%%di)\n\t"
  "pushw 2(%%di)\n\t"
  "pushw (%%di)\n\t"
  "call __adddf3\n\t"
  "addw $16, %%sp\n\t"
  "movw -20(%%bp), %%di\n\t"
  "movw %%ax, (%%di)\n\t"
  "movw %%dx, 2(%%di)\n\t"
  "movw %%cx, 4(%%di)\n\t"
  "movw %%bx, 6(%%di)\n\t"
  "addw $8, -14(%%bp)\n\t"
  "addw $8, -16(%%bp)\n\t"
  "addw $8, -18(%%bp)\n\t"
  "decw -4(%%bp)\n\t"
  "jz 10f\n\t"
  "jmp 4b\n"
  "10:\n\t"
  "addw $8, -20(%%bp)\n\t"
  "addw $8, -26(%%bp)\n\t"
  "decw -2(%%bp)\n\t"
  "jmp 2b\n"
  "20:\n\t"
  "movw %%bp, %%sp\n\t"
  "popw %%bp\n\t"
  "addw $2, %%sp\n\t"
  :
  : [mb] "m" (m), [nb] "m" (n),
    [Ab] "m" (A), [Sb] "m" (s), [Qb] "m" (q),
    [Pb] "m" (p), [Rb] "m" (r)
  : "cc", "memory", "ax", "bx", "cx", "dx", "si", "di");
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
