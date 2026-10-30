void ref_histogram(long n, const long *data, long nbins, long *hist)
{
    for (long b = 0; b < nbins - 1; b++)
        hist[b] = 0;
    for (long i = 0; i < n; i++)
        hist[data[i]]++;
}
