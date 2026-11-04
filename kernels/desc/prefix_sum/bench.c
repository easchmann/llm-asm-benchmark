#include "bench.h"

void prefix_sum(long n, const long *a, long *out);
void ref_prefix_sum(long n, const long *a, long *out);

static long n;
static long *a, *out;

static void setup(int large)
{
    n = large ? 16L << 20 : 8192;
    a = larr("a", n, IN);
    for (long i = 0; i < n; i++)
        a[i] = uniform(-1000, 1000);
    out = larr("out", n, OUT);
}

static void call(int ref)
{
    if (ref)
        ref_prefix_sum(n, a, out);
    else
        CALL(prefix_sum, n, a, out);
}
