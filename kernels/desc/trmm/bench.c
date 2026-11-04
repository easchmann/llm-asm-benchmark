#include "bench.h"

void trmm(long m, long n, double alpha, const double *A, double *B);
void ref_trmm(long m, long n, double alpha, const double *A, double *B);

static long m, n;
static double *A, *B;

static void setup(int large)
{
    m = large ? 1200 : 96;
    n = large ? 1300 : 112;
    A = darr("A", m * m, IN);
    B = darr("B", m * n, INOUT);
    batchable = 0;
}

static void call(int ref)
{
    if (ref)
        ref_trmm(m, n, 1.5, A, B);
    else
        CALL(trmm, m, n, 1.5, A, B);
}
