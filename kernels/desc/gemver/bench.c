#include "bench.h"

void gemver(long n, double alpha, double beta, double *A, const double *u1, const double *v1, const double *u2,
            const double *v2, double *w, double *x, const double *y, const double *z);
void ref_gemver(long n, double alpha, double beta, double *A, const double *u1, const double *v1,
                const double *u2, const double *v2, double *w, double *x, const double *y, const double *z);

static long n;
static double *A, *u1, *v1, *u2, *v2, *w, *x, *y, *z;

static void setup(int large)
{
    n = large ? 4000 : 120;
    A = darr("A", n * n, INOUT);
    u1 = darr("u1", n, IN);
    v1 = darr("v1", n, IN);
    u2 = darr("u2", n, IN);
    v2 = darr("v2", n, IN);
    w = darr("w", n, INOUT);
    x = darr("x", n, INOUT);
    y = darr("y", n, IN);
    z = darr("z", n, IN);
}

static void call(int ref)
{
    if (ref)
        ref_gemver(n, 1.5, -0.75, A, u1, v1, u2, v2, w, x, y, z);
    else
        CALL(gemver, n, 1.5, -0.75, A, u1, v1, u2, v2, w, x, y, z);
}
