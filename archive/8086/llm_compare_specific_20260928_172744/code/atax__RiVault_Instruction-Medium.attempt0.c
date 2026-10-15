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
    "movw $0, %%si" // i = 0
    "movw $0, %%di" // y[i] = 0, di = y
    "1:" // loop label
    "movw $0, %%ax" // y[i] = 0
    "stosw" // store y[i] = 0
    "incw %%si" // i++
    "cmpw $_PB_N, %%si" // i < n
    "jl 1b" // loop if i < n

    "movw $0, %%si" // i = 0
    "2:" // loop label
    "movw $_PB_N, %%cx" // cx = n
    "movw $0, %%ax" // tmp[i] = 0
    "3:" // loop label
    "movw %%ax, -4(%%bp) \n\t" // tmp[i]
    "movw $0, %%dx" // j = 0
    "4:" // loop label
    "pushw %%si \n\t" // save i
    "pushw %%cx \n\t" // save n
    "movw %%dx, %%si" // j
    "imulw $_PB_M, %%si \n\t" // si = j * m
    "addw %%si, %%bp \n\t" // bp = i * n + j
    "movzwl A(%%bp), %%ax \n\t" // ax = A[i][j]
    "popw %%cx \n\t" // restore n
    "popw %%si \n\t" // restore i
    "movzwl x(%%dx), %%bx \n\t" // bx = x[j]
    "imulw %%ax, %%bx \n\t" // bx = A[i][j] * x[j]
    "addw %%bx, -4(%%bp) \n\t" // tmp[i] += A[i][j] * x[j]
    "incw %%dx \n\t" // j++
    "loop 4b" // loop if j < n

    "movw $0, %%dx" // j = 0
    "5:" // loop label
    "pushw %%si \n\t" // save i
    "pushw %%cx \n\t" // save n
    "movw %%dx, %%si" // j
    "imulw $_PB_M, %%si \n\t" // si = j * m
    "addw %%si, %%bp \n\t" // bp = i * n + j
    "movzwl A(%%bp), %%ax \n\t" // ax = A[i][j]
    "popw %%cx \n\t" // restore n
    "popw %%si \n\t" // restore i
    "movzwl -4(%%bp), %%bx \n\t" // bx = tmp[i]
    "imulw %%ax, %%bx \n\t" // bx = A[i][j] * tmp[i]
    "addw %%bx, -2(%%di) \n\t" // y[j] += A[i][j] * tmp[i]
    "incw %%dx \n\t" // j++
    "loop 5b" // loop if j < n
    "incw %%si" // i++
    "cmpw $_PB_M, %%si" // i < m
    "jl 2b" // loop if i < m
    :
    "m" (m), "m" (n), "m" (A), "m" (x), "m" (y), "m" (tmp)
    : "si", "di", "ax", "bx", "cx", "dx", "bp"
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
