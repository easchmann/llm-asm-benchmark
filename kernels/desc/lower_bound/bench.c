#include "bench.h"

void lower_bound(long n, const long *sorted, long q, const long *keys, long *idx);
void ref_lower_bound(long n, const long *sorted, long q, const long *keys, long *idx);

static long n, q;
static long *sorted, *keys, *idx;

static void setup(int large)
{
    n = large ? 32L << 20 : 16384;
    q = large ? 65536 : 4096;
    sorted = larr("sorted", n, IN);
    sorted[0] = -50;
    for (long i = 1; i < n; i++)
        sorted[i] = sorted[i - 1] + uniform(0, 3);
    keys = larr("keys", q, IN);
    for (long i = 0; i < q; i++)
        keys[i] = uniform(sorted[0] - 3, sorted[n - 1] + 3);
    idx = larr("idx", q, OUT);
}

static void call(int ref)
{
    if (ref)
        ref_lower_bound(n, sorted, q, keys, idx);
    else
        CALL(lower_bound, n, sorted, q, keys, idx);
}
