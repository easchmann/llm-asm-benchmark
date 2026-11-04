#include "bench.h"

void axpy(long n, double a, const double *x, double *y);
void ref_axpy(long n, double a, const double *x, double *y);

static long n;
static double *x, *y;

static void setup(int large)
{
    n = large ? 16L << 20 : 8192;
    x = darr("x", n, IN);
    y = darr("y", n, INOUT);
}

static void call(int ref)
{
    if (ref)
        ref_axpy(n, 1.5, x, y);
    else
        CALL(axpy, n, 1.5, x, y);
}
