#include "bench.h"

void mm2(long ni, long nj, long nk, long nl, double alpha, double beta, double *tmp,
         const double *A, const double *B, const double *C, double *D);
void ref_2mm(long ni, long nj, long nk, long nl, double alpha, double beta, double *tmp,
             const double *A, const double *B, const double *C, double *D);

static long ni, nj, nk, nl;
static double *tmp, *A, *B, *C, *D;

static void setup(int large)
{
    ni = large ? 800 : 64;
    nj = large ? 850 : 72;
    nk = large ? 750 : 56;
    nl = large ? 900 : 80;
    tmp = darr("tmp", ni * nj, SCRATCH);
    A = darr("A", ni * nk, IN);
    B = darr("B", nk * nj, IN);
    C = darr("C", nj * nl, IN);
    D = darr("D", ni * nl, INOUT);
}

static void call(int ref)
{
    if (ref)
        ref_2mm(ni, nj, nk, nl, 1.5, -0.75, tmp, A, B, C, D);
    else
        CALL(mm2, ni, nj, nk, nl, 1.5, -0.75, tmp, A, B, C, D);
}
