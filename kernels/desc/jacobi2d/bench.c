#include "bench.h"

void jacobi2d(long tsteps, long n, double *A, double *B);
void ref_jacobi2d(long tsteps, long n, double *A, double *B);

static long tsteps, n;
static double *A, *B;

static void setup(int large)
{
    tsteps = large ? 2 : 10;
    n = large ? 4000 : 120;
    A = darr("A", n * n, INOUT);
    B = darr("B", n * n, INOUT);
}

static void call(int ref)
{
    if (ref)
        ref_jacobi2d(tsteps, n, A, B);
    else
        CALL(jacobi2d, tsteps, n, A, B);
}
