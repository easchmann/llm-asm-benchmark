#include "bench.h"

void heat3d(long tsteps, long n, double *A, double *B);
void ref_heat3d(long tsteps, long n, double *A, double *B);

static long tsteps, n;
static double *A, *B;

static void setup(int large)
{
    tsteps = large ? 2 : 10;
    n = large ? 250 : 24;
    A = darr("A", n * n * n, INOUT);
    B = darr("B", n * n * n, INOUT);
}

static void call(int ref)
{
    if (ref)
        ref_heat3d(tsteps, n, A, B);
    else
        CALL(heat3d, tsteps, n, A, B);
}
