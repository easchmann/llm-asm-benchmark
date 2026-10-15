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
__asm__ __volatile__ (
"	push %%bp\n"
"	mov %%sp, %%bp\n"
"	push %%bx\n"
"	push %%si\n"
"	push %%di\n"
"	mov %[m], %%bx\n"       /* bx = m */
"	mov %[n], %%cx\n"       /* cx = n */
"	mov %[y], %%si\n"       /* si = y pointer */
"	shl $1, %%cx\n"         /* cx = n*2 (data type size) */
"	add %%cx, %%si\n"       /* si = y + n*2 */
"	mov %%si, %%di\n"       /* di = end of y */
"	mov %[y], %%si\n"       /* si = start of y */
"1:	cmp %%si, %%di\n"
"	je 2f\n"
"	movw $0, (%%si)\n"
"	add $2, %%si\n"
"	jmp 1b\n"
"2:	mov %[m], %%cx\n"       /* cx = m */
"	mov %[tmp], %%si\n"     /* si = tmp pointer */
"	mov %[y], %%di\n"       /* di = y pointer */
"3:	mov %[n], %%dx\n"       /* dx = n */
"	shl $1, %%dx\n"         /* dx = n*2 */
"	add %%dx, %%di\n"       /* di = y + n*2 (end of y) */
"	mov %[y], %%di\n"       /* di = start of y */
"	movw $0, (%%si)\n"      /* tmp[i] = 0 */
"	push %%si\n"            /* save tmp pointer */
"	push %%di\n"            /* save y pointer */
"	mov %[A], %%bx\n"       /* bx = A pointer */
"	mov %[x], %%dx\n"       /* dx = x pointer */
"	mov %[n], %%cx\n"       /* cx = n */
"5:	movw (%%bx), %%ax\n"    /* A[i][j] */
"	movw (%%dx), %%dx\n"    /* x[j] */
"	imul %%dx, %%ax\n"      /* A[i][j]*x[j] */
"	pop %%si\n"             /* si = tmp pointer */
"	addw %%ax, (%%si)\n"    /* tmp[i] += ... */
"	push %%si\n"            /* save tmp pointer */
"	add $2, %%bx\n"         /* next A element */
"	add $2, %%dx\n"         /* next x element */
"	loop 5b\n"
"	pop %%si\n"             /* si = tmp pointer */
"	movw (%%si), %%ax\n"    /* ax = tmp[i] */
"	push %%ax\n"            /* save tmp[i] on stack */
"	mov %[A], %%bx\n"       /* bx = A pointer */
"	mov %[y], %%di\n"       /* di = y pointer */
"	mov %[n], %%cx\n"       /* cx = n */
"6:	movw (%%bx), %%dx\n"    /* A[i][j] */
"	pop %%ax\n"             /* ax = tmp[i] */
"	push %%ax\n"            /* save tmp[i] */
"	imul %%dx, %%ax\n"      /* A[i][j]*tmp[i] */
"	addw %%ax, (%%di)\n"    /* y[j] += ... */
"	add $2, %%bx\n"         /* next A element */
"	add $2, %%di\n"         /* next y element */
"	loop 6b\n"
"	pop %%ax\n"             /* discard saved tmp[i] */
"	add $2, %%si\n"         /* next tmp element */
"	dec %%cx\n"             /* decrement loop counter (m) */
"	jnz 3b\n"
"	pop %%di\n"
"	pop %%si\n"
"	pop %%bx\n"
"	pop %%bp\n"
: /* no outputs */
: [m] "m" (m), [n] "m" (n), [A] "m" (A), [x] "m" (x), [y] "m" (y), [tmp] "m" (tmp)
: "memory"
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
