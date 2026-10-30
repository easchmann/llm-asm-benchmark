#include "bench.h"

void bicg(long m, long n, const double *A, double *s, double *q, const double *p, const double *r);
void ref_bicg(long m, long n, const double *A, double *s, double *q, const double *p, const double *r);

static long m, n;
static double *A, *s, *q, *p, *r;

static void setup(int large)
{
    m = large ? 4200 : 130;
    n = large ? 3800 : 110;
    A = darr("A", n * m, IN);
    s = darr("s", m, OUT);
    q = darr("q", n, OUT);
    p = darr("p", m, IN);
    r = darr("r", n, IN);
}

static void call(int ref)
{
    if (ref)
        ref_bicg(m, n, A, s, q, p, r);
    else
        CALL(bicg, m, n, A, s, q, p, r);
}
