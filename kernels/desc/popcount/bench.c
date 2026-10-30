#include "bench.h"

void popcount(long n, const unsigned long *a, long *counts);
void ref_popcount(long n, const unsigned long *a, long *counts);

static long n;
static unsigned long *a;
static long *counts;

static void setup(int large)
{
    n = large ? 16L << 20 : 8192;
    a = (unsigned long *)larr("a", n, IN);
    for (long i = 0; i < n; i++)
        a[i] = next();
    counts = larr("counts", n, OUT);
}

static void call(int ref)
{
    if (ref)
        ref_popcount(n, a, counts);
    else
        CALL(popcount, n, a, counts);
}
