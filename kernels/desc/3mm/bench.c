#include "bench.h"

void mm3(long ni, long nj, long nk, long nl, long nm, double *E, const double *A, const double *B,
         double *F, const double *C, const double *D, double *G);
void ref_3mm(long ni, long nj, long nk, long nl, long nm, double *E, const double *A, const double *B,
             double *F, const double *C, const double *D, double *G);

static long ni, nj, nk, nl, nm;
static double *E, *A, *B, *F, *C, *D, *G;

static void setup(int large)
{
    ni = large ? 700 : 64;
    nj = large ? 750 : 72;
    nk = large ? 650 : 56;
    nl = large ? 800 : 80;
    nm = large ? 680 : 60;
    E = darr("E", ni * nj, SCRATCH);
    A = darr("A", ni * nk, IN);
    B = darr("B", nk * nj, IN);
    F = darr("F", nj * nl, SCRATCH);
    C = darr("C", nj * nm, IN);
    D = darr("D", nm * nl, IN);
    G = darr("G", ni * nl, OUT);
}

static void call(int ref)
{
    if (ref)
        ref_3mm(ni, nj, nk, nl, nm, E, A, B, F, C, D, G);
    else
        CALL(mm3, ni, nj, nk, nl, nm, E, A, B, F, C, D, G);
}
