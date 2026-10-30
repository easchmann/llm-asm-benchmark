#include "bench.h"

void sort8(long n, long *a);
void ref_sort8(long n, long *a);

static long n;
static long *a;

static void setup(int large)
{
    n = large ? 16L << 20 : 8192;
    a = larr("a", n, INOUT);
    for (long i = 0; i < n; i++)
        a[i] = uniform(-1000, 1000);
    batchable = 0;
}

static void call(int ref)
{
    if (ref)
        ref_sort8(n, a);
    else
        CALL(sort8, n, a);
}
