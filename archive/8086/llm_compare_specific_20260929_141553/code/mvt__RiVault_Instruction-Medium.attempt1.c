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
  "movw %1, %%cx;" // n
  "movw %2, %%si;" // x1
  "movw %3, %%di;" // A
  "movw %4, %%bp;" // y_1
  "xorw %%ax, %%ax;" // x1[i] = 0
  "movw $0, %%i;" // i = 0
  "jmp 1f;" // goto loop
  "0: movw %%cx, %%dx;" // dx = n
  "subw $1, %%dx;" // dx = n-1
  "jz 3f;" // if i == n goto end
  "movw $0, %%j;" // j = 0
  "1: cmpw %%cx, %%j;" // j < n
  "jge 3f;" // if j >= n goto end
  "movw %%j, %%ax;" // ax = j
  "imulw %%cx, %%ax;" // ax = j*n
  "addw %%i, %%ax;" // ax = i*n + j
  "movw %%ax, %%dx;" // dx = i*n + j
  "movzwl %%dx, %%dx;" // zero-extend dx
  "movw %3(%%dx), %%dx;" // dx = A[i][j]
  "imulw %4(%%j), %%dx;" // dx = A[i][j]*y_1[j]
  "addw %%dx, %%si;" // x1[i] += A[i][j]*y_1[j]
  "incw %%j;" // j++
  "jmp 1b;" // loop
  "3: incw %%i;" // i++
  "jmp 0b;" // loop

  "movw %5, %%si;" // x2
  "movw %3, %%di;" // A
  "movw %6, %%bp;" // y_2
  "xorw %%ax, %%ax;" // x2[i] = 0
  "movw $0, %%i;" // i = 0
  "jmp 4f;" // goto loop
  "4: movw %%cx, %%dx;" // dx = n
  "subw $1, %%dx;" // dx = n-1
  "jz 6f;" // if i == n goto end
  "movw $0, %%j;" // j = 0
  "5: cmpw %%cx, %%j;" // j < n
  "jge 6f;" // if j >= n goto end
  "movw %%j, %%ax;" // ax = j
  "imulw %%cx, %%ax;" // ax = j*n
  "addw %%j, %%ax;" // ax = j*n + j
  "movw %%ax, %%dx;" // dx = j*n + j
  "movzwl %%dx, %%dx;" // zero-extend dx
  "movw %3(%%dx), %%dx;" // dx = A[j][i]
  "imulw %6(%%j), %%dx;" // dx = A[j][i]*y_2[j]
  "addw %%dx, %%si;" // x2[i] += A[j][i]*y_2[j]
  "incw %%j;" // j++
  "jmp 5b;" // loop
  "6: incw %%i;" // i++
  "jmp 4b;" // loop
  :
    [n] "r" (n),
    [x1] "r" (x1),
    [A] "r" (A),
    [y_1] "r" (y_1),
    [x2] "r" (x2),
    [y_2] "r" (y_2)
  :
    [i] "r" (i),
    [j] "r" (j)
  :
    "ax", "cx", "dx", "si", "di", "bp"
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
