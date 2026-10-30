#include "bench.h"

void jacobi1d(long tsteps, long n, double *A, double *B);
void ref_jacobi1d(long tsteps, long n, double *A, double *B);

static long tsteps, n;
static double *A, *B;

static void setup(int large)
{
    tsteps = large ? 2 : 10;
    n = large ? 16L << 20 : 16384;
    A = darr("A", n, INOUT);
    B = darr("B", n, INOUT);
}

static void call(int ref)
{
    if (ref)
        ref_jacobi1d(tsteps, n, A, B);
    else
        CALL(jacobi1d, tsteps, n, A, B);
}
