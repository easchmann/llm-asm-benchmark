#include "bench.h"

void scal(long n, double a, double *x);
void ref_scal(long n, double a, double *x);

static long n;
static double *x;

static void setup(int large)
{
    n = large ? 32L << 20 : 16384;
    x = darr("x", n, INOUT);
}

static void call(int ref)
{
    if (ref)
        ref_scal(n, 1.5, x);
    else
        CALL(scal, n, 1.5, x);
}
