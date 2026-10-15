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
        "push %%bp\n\t"
        "mov %%sp, %%bp\n\t"
        "mov %[M], %%cx\n\t"
        "mov %[N], %%dx\n\t"
        "shl $1, %%dx\n\t"
        "mov %[y], %%di\n\t"
        "mov %%dx, %%ax\n\t"
        "shr $1, %%ax\n\t"
        "test %%ax, %%ax\n\t"
        "jz .Latax_y_zero_done\n\t"
        "xor %%si, %%si\n\t"
        ".Latax_y_zero_loop:\n\t"
        "movw $0, (%%di, %%si)\n\t"
        "add $2, %%si\n\t"
        "cmp %%ax, %%si\n\t"
        "jl .Latax_y_zero_loop\n\t"
        ".Latax_y_zero_done:\n\t"

        "mov %[tmp], %%di\n\t"
        "mov %[A], %%si\n\t"
        "mov %[x], %%bx\n\t"
        "xor %%cx, %%cx\n\t"
        ".Latax_i_loop:\n\t"
        "cmp %[M], %%cx\n\t"
        "jge .Latax_done\n\t"

        "movw $0, (%%di, %%cx, 2)\n\t"

        "push %%cx\n\t"
        "push %%di\n\t"
        "push %%si\n\t"
        "push %%bx\n\t"
        "mov %%cx, %%ax\n\t"
        "shl $1, %%ax\n\t"
        "shl $1, %%ax\n\t"
        "add %%ax, %%si\n\t"
        "mov %[N], %%cx\n\t"
        "shl $1, %%cx\n\t"
        "xor %%dx, %%dx\n\t"
        ".Latax_j_loop:\n\t"
        "mov (%%si, %%dx), %%ax\n\t"
        "imul (%%bx, %%dx)\n\t"
        "add %%ax, (%%di)\n\t"
        "add $2, %%dx\n\t"
        "cmp %%cx, %%dx\n\t"
        "jl .Latax_j_loop\n\t"

        "pop %%bx\n\t"
        "pop %%si\n\t"
        "pop %%di\n\t"
        "pop %%cx\n\t"

        "push %%cx\n\t"
        "push %%di\n\t"
        "push %%si\n\t"
        "push %%bx\n\t"
        "mov %%cx, %%ax\n\t"
        "shl $1, %%ax\n\t"
        "shl $1, %%ax\n\t"
        "add %%ax, %%si\n\t"
        "mov %[N], %%cx\n\t"
        "shl $1, %%cx\n\t"
        "xor %%dx, %%dx\n\t"
        "mov (%%di, %%cx, 2), %%ax\n\t"
        ".Latax_j2_loop:\n\t"
        "mov (%%si, %%dx), %%cx\n\t"
        "imul %%ax\n\t"
        "add %%ax, (%%di, %%dx)\n\t"
        "add $2, %%dx\n\t"
        "cmp %[N_byte], %%dx\n\t"
        "jl .Latax_j2_loop\n\t"

        "pop %%bx\n\t"
        "pop %%si\n\t"
        "pop %%di\n\t"
        "pop %%cx\n\t"

        "inc %%cx\n\t"
        "jmp .Latax_i_loop\n\t"
        ".Latax_done:\n\t"
        "pop %%bp\n\t"
        :
        : [M] "r" (m), [N] "r" (n), [N_byte] "r" (n*2), [A] "r" (A_ptr), [x] "r" (x_ptr), [y] "r" (y_ptr), [tmp] "r" (tmp_ptr)
        : "ax", "bx", "cx", "dx", "si", "di", "bp", "memory"
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
