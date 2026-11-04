#include "bench.h"

double nrm2(long n, const double *x);
double ref_nrm2(long n, const double *x);

static long n;
static double *x, *result;

static void setup(int large)
{
    n = large ? 32L << 20 : 16384;
    x = darr("x", n, IN);
    result = darr("result", 1, OUT);
}

static void call(int ref)
{
    result[0] = ref ? ref_nrm2(n, x) : CALL(nrm2, n, x);
}
