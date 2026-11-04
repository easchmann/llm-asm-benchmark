#include "bench.h"

void mvt(long n, double *x1, double *x2, const double *y_1, const double *y_2, const double *A);
void ref_mvt(long n, double *x1, double *x2, const double *y_1, const double *y_2, const double *A);

static long n;
static double *x1, *x2, *y_1, *y_2, *A;

static void setup(int large)
{
    n = large ? 4000 : 120;
    x1 = darr("x1", n, INOUT);
    x2 = darr("x2", n, INOUT);
    y_1 = darr("y_1", n, IN);
    y_2 = darr("y_2", n, IN);
    A = darr("A", n * n, IN);
}

static void call(int ref)
{
    if (ref)
        ref_mvt(n, x1, x2, y_1, y_2, A);
    else
        CALL(mvt, n, x1, x2, y_1, y_2, A);
}
