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
__asm__ __volatile__ (
        "movw %0, %%ax\n\t"
        "movw %1, %%bx\n\t"
        "pushw %%ax\n\t"
        "pushw %%bx\n\t"
        "pushw %2\n\t"
        "pushw %3\n\t"
        "pushw %4\n\t"
        "pushw %5\n\t"
        "pushw %6\n\t"
        "pushw %%bp\n\t"
        "movw %%sp, %%bp\n\t"
        "pushw %2\n\t"
        "pushw $0\n\t"
        "movw 14(%%bp), %%cx\n\t"
        "jcxz .Lz_end\n\t"
        "movw 8(%%bp), %%si\n\t"
        ".Lz_el:\n\t"
        "movw %8, %%ax\n\t"
        ".Lz_w:\n\t"
        "movw $0, (%%si)\n\t"
        "addw $2, %%si\n\t"
        "decw %%ax\n\t"
        "jnz .Lz_w\n\t"
        "loop .Lz_el\n\t"
        ".Lz_end:\n\t"
        "movw %7, %%ax\n\t"
        "cmpw $8, %%ax\n\t"
        "je .Ld_main\n\t"
        "cmpw $4, %%ax\n\t"
        "je .Lf_main\n\t"
        "cmpw $2, %%ax\n\t"
        "je .Li_main\n\t"
        "jmp .Lall\n\t"
        ".Ld_main:\n\t"
        "movw 12(%%bp), %%ax\n\t"
        "testw %%ax, %%ax\n\t"
        "jz .Ld_done\n\t"
        ".Ld_out:\n\t"
        "movw 14(%%bp), %%dx\n\t"
        "movb $3, %%cl\n\t"
        "shlw %%cl, %%dx\n\t"
        "movw -2(%%bp), %%ax\n\t"
        "movw %%ax, %%di\n\t"
        "movw %%dx, %%cx\n\t"
        "addw %%di, %%cx\n\t"
        "cmpw %%cx, %%di\n\t"
        "je .Ld_p1skip\n\t"
        "movw -4(%%bp), %%bx\n\t"
        "movb $3, %%cl\n\t"
        "shlw %%cl, %%bx\n\t"
        "addw 2(%%bp), %%bx\n\t"
        "movw 8(%%bp), %%si\n\t"
        ".Ld_p1:\n\t"
        "fldl (%%bx)\n\t"
        "fmull (%%di)\n\t"
        "faddl (%%si)\n\t"
        "fstpl (%%si)\n\t"
        "addw $8, %%di\n\t"
        "addw $8, %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "jne .Ld_p1\n\t"
        ".Ld_p1skip:\n\t"
        "movw -4(%%bp), %%ax\n\t"
        "movb $3, %%cl\n\t"
        "shlw %%cl, %%ax\n\t"
        "addw 4(%%bp), %%ax\n\t"
        "movw %%ax, %%bx\n\t"
        "movw -2(%%bp), %%ax\n\t"
        "movw %%ax, %%di\n\t"
        "movw %%di, %%cx\n\t"
        "addw %%dx, %%cx\n\t"
        "movw 6(%%bp), %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "je .Ld_qz\n\t"
        "fldl (%%di)\n\t"
        "fmull (%%si)\n\t"
        "fstpl (%%bx)\n\t"
        "addw $8, %%di\n\t"
        "addw $8, %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "je .Ld_p2x\n\t"
        ".Ld_p2:\n\t"
        "fldl (%%di)\n\t"
        "fmull (%%si)\n\t"
        "faddl (%%bx)\n\t"
        "fstpl (%%bx)\n\t"
        "addw $8, %%di\n\t"
        "addw $8, %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "jne .Ld_p2\n\t"
        ".Ld_qz:\n\t"
        "fldz\n\t"
        "fstpl (%%bx)\n\t"
        ".Ld_p2x:\n\t"
        "movw -2(%%bp), %%ax\n\t"
        "addw %%dx, %%ax\n\t"
        "movw %%ax, -2(%%bp)\n\t"
        "incw -4(%%bp)\n\t"
        "movw -4(%%bp), %%ax\n\t"
        "cmpw 12(%%bp), %%ax\n\t"
        "jb .Ld_out\n\t"
        ".Ld_done:\n\t"
        "jmp .Lall\n\t"
        ".Lf_main:\n\t"
        "movw 12(%%bp), %%ax\n\t"
        "testw %%ax, %%ax\n\t"
        "jz .Lf_done\n\t"
        ".Lf_out:\n\t"
        "movw 14(%%bp), %%dx\n\t"
        "movb $2, %%cl\n\t"
        "shlw %%cl, %%dx\n\t"
        "movw -2(%%bp), %%ax\n\t"
        "movw %%ax, %%di\n\t"
        "movw %%dx, %%cx\n\t"
        "addw %%di, %%cx\n\t"
        "cmpw %%cx, %%di\n\t"
        "je .Lf_p1skip\n\t"
        "movw -4(%%bp), %%bx\n\t"
        "movb $2, %%cl\n\t"
        "shlw %%cl, %%bx\n\t"
        "addw 2(%%bp), %%bx\n\t"
        "movw 8(%%bp), %%si\n\t"
        ".Lf_p1:\n\t"
        "flds (%%bx)\n\t"
        "fmuls (%%di)\n\t"
        "fadds (%%si)\n\t"
        "fstps (%%si)\n\t"
        "addw $4, %%di\n\t"
        "addw $4, %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "jne .Lf_p1\n\t"
        ".Lf_p1skip:\n\t"
        "movw -4(%%bp), %%ax\n\t"
        "movb $2, %%cl\n\t"
        "shlw %%cl, %%ax\n\t"
        "addw 4(%%bp), %%ax\n\t"
        "movw %%ax, %%bx\n\t"
        "movw -2(%%bp), %%ax\n\t"
        "movw %%ax, %%di\n\t"
        "movw %%di, %%cx\n\t"
        "addw %%dx, %%cx\n\t"
        "movw 6(%%bp), %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "je .Lf_qz\n\t"
        "flds (%%di)\n\t"
        "fmuls (%%si)\n\t"
        "fstps (%%bx)\n\t"
        "addw $4, %%di\n\t"
        "addw $4, %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "je .Lf_p2x\n\t"
        ".Lf_p2:\n\t"
        "flds (%%di)\n\t"
        "fmuls (%%si)\n\t"
        "fadds (%%bx)\n\t"
        "fstps (%%bx)\n\t"
        "addw $4, %%di\n\t"
        "addw $4, %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "jne .Lf_p2\n\t"
        ".Lf_qz:\n\t"
        "fldz\n\t"
        "fstps (%%bx)\n\t"
        ".Lf_p2x:\n\t"
        "movw -2(%%bp), %%ax\n\t"
        "addw %%dx, %%ax\n\t"
        "movw %%ax, -2(%%bp)\n\t"
        "incw -4(%%bp)\n\t"
        "movw -4(%%bp), %%ax\n\t"
        "cmpw 12(%%bp), %%ax\n\t"
        "jb .Lf_out\n\t"
        ".Lf_done:\n\t"
        "jmp .Lall\n\t"
        ".Li_main:\n\t"
        "movw 12(%%bp), %%ax\n\t"
        "testw %%ax, %%ax\n\t"
        "jz .Li_done\n\t"
        ".Li_out:\n\t"
        "movw 14(%%bp), %%dx\n\t"
        "shlw $1, %%dx\n\t"
        "movw -2(%%bp), %%ax\n\t"
        "movw %%ax, %%di\n\t"
        "movw %%dx, %%cx\n\t"
        "addw %%di, %%cx\n\t"
        "movw 8(%%bp), %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "je .Li_p1skip\n\t"
        "movw -4(%%bp), %%bx\n\t"
        "shlw $1, %%bx\n\t"
        "addw 2(%%bp), %%bx\n\t"
        ".Li_p1:\n\t"
        "movw (%%bx), %%ax\n\t"
        "mulw (%%di)\n\t"
        "addw %%ax, (%%si)\n\t"
        "addw $2, %%di\n\t"
        "addw $2, %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "jne .Li_p1\n\t"
        ".Li_p1skip:\n\t"
        "movw -4(%%bp), %%ax\n\t"
        "shlw $1, %%ax\n\t"
        "addw 4(%%bp), %%ax\n\t"
        "movw %%ax, %%bx\n\t"
        "movw -2(%%bp), %%ax\n\t"
        "movw %%ax, %%di\n\t"
        "movw %%di, %%cx\n\t"
        "addw %%dx, %%cx\n\t"
        "movw 6(%%bp), %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "je .Li_qz\n\t"
        "movw (%%di), %%ax\n\t"
        "mulw (%%si)\n\t"
        "movw %%ax, (%%bx)\n\t"
        "addw $2, %%di\n\t"
        "addw $2, %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "je .Li_p2x\n\t"
        ".Li_p2:\n\t"
        "movw (%%di), %%ax\n\t"
        "mulw (%%si)\n\t"
        "addw %%ax, (%%bx)\n\t"
        "addw $2, %%di\n\t"
        "addw $2, %%si\n\t"
        "cmpw %%cx, %%di\n\t"
        "jne .Li_p2\n\t"
        ".Li_qz:\n\t"
        "movw $0, (%%bx)\n\t"
        ".Li_p2x:\n\t"
        "movw 14(%%bp), %%dx\n\t"
        "shlw $1, %%dx\n\t"
        "movw -2(%%bp), %%ax\n\t"
        "addw %%dx, %%ax\n\t"
        "movw %%ax, -2(%%bp)\n\t"
        "incw -4(%%bp)\n\t"
        "movw -4(%%bp), %%ax\n\t"
        "cmpw 12(%%bp), %%ax\n\t"
        "jb .Li_out\n\t"
        ".Li_done:\n\t"
        ".Lall:\n\t"
        "fwait\n\t"
        "movw %%bp, %%sp\n\t"
        "popw %%bp\n\t"
        "popw %%ax\n\t"
        "popw %%ax\n\t"
        "popw %%ax\n\t"
        "popw %%ax\n\t"
        "popw %%ax\n\t"
        "popw %%ax\n\t"
        "popw %%ax\n"
        :
        : "m" (m), "m" (n), "r" (A), "r" (s), "r" (p),
          "r" (q), "r" (r),
          "i" ((int) sizeof (DATA_TYPE)),
          "i" ((int) (sizeof (DATA_TYPE) / 2))
        : "ax", "bx", "cx", "dx", "si", "di", "cc", "memory");
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
