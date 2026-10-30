#include "bench.h"

void histogram(long n, const long *data, long nbins, long *hist);
void ref_histogram(long n, const long *data, long nbins, long *hist);

static long n, nbins = 256;
static long *data, *hist;

static void setup(int large)
{
    n = large ? 32L << 20 : 16384;
    data = larr("data", n, IN);
    for (long i = 0; i < n; i++)
        data[i] = uniform(0, nbins - 1);
    hist = larr("hist", nbins, OUT);
}

static void call(int ref)
{
    if (ref)
        ref_histogram(n, data, nbins, hist);
    else
        CALL(histogram, n, data, nbins, hist);
}
