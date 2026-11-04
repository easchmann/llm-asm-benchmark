#include "bench.h"

void gesummv(long n, double alpha, double beta, const double *A, const double *B, double *tmp, const double *x,
             double *y);
void ref_gesummv(long n, double alpha, double beta, const double *A, const double *B, double *tmp,
                 const double *x, double *y);

static long n;
static double *A, *B, *tmp, *x, *y;

static void setup(int large)
{
    n = large ? 3000 : 90;
    A = darr("A", n * n, IN);
    B = darr("B", n * n, IN);
    tmp = darr("tmp", n, SCRATCH);
    x = darr("x", n, IN);
    y = darr("y", n, OUT);
}

static void call(int ref)
{
    if (ref)
        ref_gesummv(n, 1.5, -0.75, A, B, tmp, x, y);
    else
        CALL(gesummv, n, 1.5, -0.75, A, B, tmp, x, y);
}
