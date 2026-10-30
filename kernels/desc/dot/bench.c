#include "bench.h"

double dot(long n, const double *x, const double *y);
double ref_dot(long n, const double *x, const double *y);

static long n;
static double *x, *y, *result;

static void setup(int large)
{
    n = large ? 16L << 20 : 8192;
    x = darr("x", n, IN);
    y = darr("y", n, IN);
    result = darr("result", 1, OUT);
}

static void call(int ref)
{
    result[0] = ref ? ref_dot(n, x, y) : CALL(dot, n, x, y);
}
