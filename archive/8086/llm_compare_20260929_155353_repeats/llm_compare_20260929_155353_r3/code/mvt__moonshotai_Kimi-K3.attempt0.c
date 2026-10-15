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
#if defined(DATA_TYPE_IS_INT)
	"xorw %%dx, %%dx\n"
	"0:\n"
	"cmpw $%[pb_n], %%dx\n"
	"jge 3f\n"
	"movw %[x1], %%di\n"
	"movw %%dx, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%di\n"
	"movw %%dx, %%cx\n"
	"movw %%dx, %%ax\n"
	"mulw %[n]\n"
	"movw %%cx, %%dx\n"
	"shlw $1, %%ax\n"
	"addw %[A], %%ax\n"
	"movw %%ax, %%bx\n"
	"xorw %%cx, %%cx\n"
	"1:\n"
	"cmpw $%[pb_n], %%cx\n"
	"jge 2f\n"
	"movw %[y_1], %%si\n"
	"movw %%cx, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%si\n"
	"movw (%%bx), %%ax\n"
	"pushw %%dx\n"
	"imulw (%%si)\n"
	"popw %%dx\n"
	"addw (%%di), %%ax\n"
	"movw %%ax, (%%di)\n"
	"addw $2, %%bx\n"
	"incw %%cx\n"
	"jmp 1b\n"
	"2:\n"
	"incw %%dx\n"
	"jmp 0b\n"
	"3:\n"
	"xorw %%dx, %%dx\n"
	"4:\n"
	"cmpw $%[pb_n], %%dx\n"
	"jge 7f\n"
	"movw %[x2], %%di\n"
	"movw %%dx, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%di\n"
	"movw %[A], %%bx\n"
	"movw %%dx, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%bx\n"
	"xorw %%cx, %%cx\n"
	"5:\n"
	"cmpw $%[pb_n], %%cx\n"
	"jge 6f\n"
	"movw %[y_2], %%si\n"
	"movw %%cx, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%si\n"
	"movw (%%bx), %%ax\n"
	"pushw %%dx\n"
	"imulw (%%si)\n"
	"popw %%dx\n"
	"addw (%%di), %%ax\n"
	"movw %%ax, (%%di)\n"
	"movw %[n], %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%bx\n"
	"incw %%cx\n"
	"jmp 5b\n"
	"6:\n"
	"incw %%dx\n"
	"jmp 4b\n"
	"7:\n"
#elif defined(DATA_TYPE_IS_FLOAT)
	"xorw %%dx, %%dx\n"
	"0:\n"
	"cmpw $%[pb_n], %%dx\n"
	"jge 3f\n"
	"movw %[x1], %%di\n"
	"movw %%dx, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%di\n"
	"movw %%dx, %%cx\n"
	"movw %%dx, %%ax\n"
	"mulw %[n]\n"
	"movw %%cx, %%dx\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %[A], %%ax\n"
	"movw %%ax, %%bx\n"
	"xorw %%cx, %%cx\n"
	"1:\n"
	"cmpw $%[pb_n], %%cx\n"
	"jge 2f\n"
	"movw %[y_1], %%si\n"
	"movw %%cx, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%si\n"
	"flds (%%bx)\n"
	"fmuls (%%si)\n"
	"fadds (%%di)\n"
	"fstps (%%di)\n"
	"addw $4, %%bx\n"
	"incw %%cx\n"
	"jmp 1b\n"
	"2:\n"
	"incw %%dx\n"
	"jmp 0b\n"
	"3:\n"
	"xorw %%dx, %%dx\n"
	"4:\n"
	"cmpw $%[pb_n], %%dx\n"
	"jge 7f\n"
	"movw %[x2], %%di\n"
	"movw %%dx, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%di\n"
	"movw %[A], %%bx\n"
	"movw %%dx, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%bx\n"
	"xorw %%cx, %%cx\n"
	"5:\n"
	"cmpw $%[pb_n], %%cx\n"
	"jge 6f\n"
	"movw %[y_2], %%si\n"
	"movw %%cx, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%si\n"
	"flds (%%bx)\n"
	"fmuls (%%si)\n"
	"fadds (%%di)\n"
	"fstps (%%di)\n"
	"movw %[n], %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%bx\n"
	"incw %%cx\n"
	"jmp 5b\n"
	"6:\n"
	"incw %%dx\n"
	"jmp 4b\n"
	"7:\n"
#else
	"xorw %%dx, %%dx\n"
	"0:\n"
	"cmpw $%[pb_n], %%dx\n"
	"jge 3f\n"
	"movw %[x1], %%di\n"
	"movw %%dx, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%di\n"
	"movw %%dx, %%cx\n"
	"movw %%dx, %%ax\n"
	"mulw %[n]\n"
	"movw %%cx, %%dx\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %[A], %%ax\n"
	"movw %%ax, %%bx\n"
	"xorw %%cx, %%cx\n"
	"1:\n"
	"cmpw $%[pb_n], %%cx\n"
	"jge 2f\n"
	"movw %[y_1], %%si\n"
	"movw %%cx, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%si\n"
	"fldl (%%bx)\n"
	"fmull (%%si)\n"
	"faddl (%%di)\n"
	"fstpl (%%di)\n"
	"addw $8, %%bx\n"
	"incw %%cx\n"
	"jmp 1b\n"
	"2:\n"
	"incw %%dx\n"
	"jmp 0b\n"
	"3:\n"
	"xorw %%dx, %%dx\n"
	"4:\n"
	"cmpw $%[pb_n], %%dx\n"
	"jge 7f\n"
	"movw %[x2], %%di\n"
	"movw %%dx, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%di\n"
	"movw %[A], %%bx\n"
	"movw %%dx, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%bx\n"
	"xorw %%cx, %%cx\n"
	"5:\n"
	"cmpw $%[pb_n], %%cx\n"
	"jge 6f\n"
	"movw %[y_2], %%si\n"
	"movw %%cx, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%si\n"
	"fldl (%%bx)\n"
	"fmull (%%si)\n"
	"faddl (%%di)\n"
	"fstpl (%%di)\n"
	"movw %[n], %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"shlw $1, %%ax\n"
	"addw %%ax, %%bx\n"
	"incw %%cx\n"
	"jmp 5b\n"
	"6:\n"
	"incw %%dx\n"
	"jmp 4b\n"
	"7:\n"
#endif
	:
	: [n] "m" (n),
	  [pb_n] "i" (_PB_N),
	  [x1] "m" (x1),
	  [x2] "m" (x2),
	  [y_1] "m" (y_1),
	  [y_2] "m" (y_2),
	  [A] "m" (A)
	: "ax", "bx", "cx", "dx", "si", "di", "cc", "memory");
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
