#include "bench.h"

void gemm(long ni, long nj, long nk, double alpha, double beta, double *C, const double *A, const double *B);
void ref_gemm(long ni, long nj, long nk, double alpha, double beta, double *C, const double *A, const double *B);

static long ni, nj, nk;
static double *C, *A, *B;

static void setup(int large)
{
    ni = large ? 1000 : 96;
    nj = large ? 1100 : 112;
    nk = large ? 900 : 80;
    C = darr("C", ni * nj, INOUT);
    A = darr("A", ni * nk, IN);
    B = darr("B", nk * nj, IN);
}

static void call(int ref)
{
    if (ref)
        ref_gemm(ni, nj, nk, 1.5, -0.75, C, A, B);
    else
        CALL(gemm, ni, nj, nk, 1.5, -0.75, C, A, B);
}
