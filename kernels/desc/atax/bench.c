#include "bench.h"

void atax(long m, long n, const double *A, const double *x, double *y, double *tmp);
void ref_atax(long m, long n, const double *A, const double *x, double *y, double *tmp);

static long m, n;
static double *A, *x, *y, *tmp;

static void setup(int large)
{
    m = large ? 3800 : 110;
    n = large ? 4200 : 130;
    A = darr("A", m * n, IN);
    x = darr("x", n, IN);
    y = darr("y", n, OUT);
    tmp = darr("tmp", m, SCRATCH);
}

static void call(int ref)
{
    if (ref)
        ref_atax(m, n, A, x, y, tmp);
    else
        CALL(atax, m, n, A, x, y, tmp);
}
