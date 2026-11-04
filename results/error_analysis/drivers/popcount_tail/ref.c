void ref_popcount(long n, const unsigned long *a, long *counts)
{
    for (long i = 0; i < n; i++) {
        unsigned long v = a[i];
        long c = 0;
        while (v) {
            v &= v - 1;
            c++;
        }
        counts[i] = c;
    }
}
