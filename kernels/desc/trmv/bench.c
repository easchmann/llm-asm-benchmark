#include "bench.h"

void trmv(long n, const double *L, const double *x, double *y);
void ref_trmv(long n, const double *L, const double *x, double *y);

static long n;
static double *L, *x, *y;

static void setup(int large)
{
    n = large ? 4000 : 150;
    L = darr("L", n * n, IN);
    x = darr("x", n, IN);
    y = darr("y", n, OUT);
}

static void call(int ref)
{
    if (ref)
        ref_trmv(n, L, x, y);
    else
        CALL(trmv, n, L, x, y);
}
