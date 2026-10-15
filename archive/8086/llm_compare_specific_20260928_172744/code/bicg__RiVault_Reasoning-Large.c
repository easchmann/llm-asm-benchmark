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
    "movw %0, %%cx\n\t"           /* m */
    "movw %3, %%di\n\t"           /* s */
    "xorw %%ax, %%ax\n\t"
    "1:\n\t"
    "movw %%ax, (%%di)\n\t"       /* s[i] = 0 */
    "addw $2, %%di\n\t"
    "decw %%cx\n\t"
    "jnz 1b\n\t"
    "movw %1, %%ax\n\t"           /* n */
    "testw %%ax, %%ax\n\t"
    "jnz 6f\n\t"
    "jmp 7f\n\t"                  /* n == 0: nothing more to do */
    "6:\n\t"
    "pushw %%bp\n\t"              /* save frame pointer */
    "pushw %%ax\n\t"              /* i-loop countdown = n */
    "movw %4, %%ax\n\t"
    "pushw %%ax\n\t"              /* q walker */
    "movw %6, %%ax\n\t"
    "pushw %%ax\n\t"              /* r walker */
    "movw %3, %%ax\n\t"
    "pushw %%ax\n\t"              /* s base */
    "movw %5, %%ax\n\t"
    "pushw %%ax\n\t"              /* p base */
    "movw %0, %%ax\n\t"
    "pushw %%ax\n\t"              /* m */
    "movw %2, %%bx\n\t"           /* bx = A walker */
    "4:\n\t"                      /* ---- i-loop prologue ---- */
    "popw %%cx\n\t"               /* cx = m */
    "popw %%di\n\t"               /* di = p base */
    "popw %%si\n\t"               /* si = s base */
    "popw %%dx\n\t"               /* dx = r walker */
    "popw %%ax\n\t"               /* ax = q walker */
    "pushw %%bx\n\t"
    "movw %%dx, %%bx\n\t"
    "movw (%%bx), %%bp\n\t"       /* bp = r[i] */
    "addw $2, %%bx\n\t"
    "popw %%bx\n\t"
    "addw $2, %%dx\n\t"
    "pushw %%dx\n\t"              /* park r walker (advanced) */
    "pushw %%si\n\t"              /* park s base */
    "pushw %%di\n\t"              /* park p base */
    "pushw %%cx\n\t"              /* park m */
    "pushw %%ax\n\t"              /* park q walker */
    "xorw %%dx, %%dx\n\t"
    "pushw %%dx\n\t"              /* qacc = 0 on stack */
    "jcxz 3f\n\t"                 /* m == 0: skip inner loop */
    "2:\n\t"                      /* ---- inner j loop ---- */
    "movw (%%bx), %%ax\n\t"       /* ax = A[i][j] */
    "addw $2, %%bx\n\t"
    "pushw %%ax\n\t"              /* stash A[i][j] */
    "imulw %%bp\n\t"              /* ax = r[i]*A[i][j] */
    "addw %%ax, (%%si)\n\t"       /* s[j] += ... */
    "addw $2, %%si\n\t"
    "movw (%%di), %%ax\n\t"       /* ax = p[j] */
    "addw $2, %%di\n\t"
    "popw %%dx\n\t"               /* dx = A[i][j] */
    "imulw %%dx\n\t"              /* ax = A[i][j]*p[j] */
    "popw %%dx\n\t"               /* dx = qacc */
    "addw %%ax, %%dx\n\t"
    "pushw %%dx\n\t"              /* save qacc */
    "decw %%cx\n\t"
    "jnz 2b\n\t"
    "3:\n\t"                      /* ---- i-loop epilogue ---- */
    "popw %%dx\n\t"               /* dx = qacc */
    "popw %%si\n\t"               /* si = q walker */
    "movw %%dx, (%%si)\n\t"       /* q[i] = qacc */
    "addw $2, %%si\n\t"           /* advance q walker */
    "popw %%ax\n\t"               /* ax = m */
    "popw %%cx\n\t"               /* cx = p base */
    "popw %%di\n\t"               /* di = s base */
    "popw %%bp\n\t"               /* bp = r walker */
    "popw %%dx\n\t"               /* dx = countdown */
    "decw %%dx\n\t"
    "jz 5f\n\t"
    "pushw %%dx\n\t"              /* park countdown' */
    "pushw %%si\n\t"              /* park q walker */
    "pushw %%bp\n\t"              /* park r walker */
    "pushw %%di\n\t"              /* park s base */
    "pushw %%cx\n\t"              /* park p base */
    "pushw %%ax\n\t"              /* park m */
    "jmp 4b\n\t"
    "5:\n\t"
    "popw %%bp\n\t"               /* restore frame pointer */
    "7:"
    : /* no outputs */
    : "m"(m), "m"(n), "m"(A), "m"(s), "m"(q), "m"(p), "m"(r)
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
