void ref_prefix_sum(long n, const long *a, long *out)
{
    long s = 0;
    for (long i = 0; i < n; i++) {
        out[i] = s;
        s += a[i];
    }
}
