#include "bench.h"

void syrk(long n, long m, double alpha, double beta, double *C, const double *A);
void ref_syrk(long n, long m, double alpha, double beta, double *C, const double *A);

static long n, m;
static double *C, *A;

static void setup(int large)
{
    n = large ? 1200 : 96;
    m = large ? 1000 : 80;
    C = darr("C", n * n, INOUT);
    A = darr("A", n * m, IN);
}

static void call(int ref)
{
    if (ref)
        ref_syrk(n, m, 1.5, -0.75, C, A);
    else
        CALL(syrk, n, m, 1.5, -0.75, C, A);
}
