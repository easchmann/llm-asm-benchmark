void ref_lower_bound(long n, const long *sorted, long q, const long *keys, long *idx)
{
    for (long t = 0; t < q; t++) {
        long lo = 0, hi = n;
        while (lo < hi) {
            long mid = lo + (hi - lo) / 2;
            if (sorted[mid] <= keys[t])
                lo = mid + 1;
            else
                hi = mid;
        }
        idx[t] = lo;
    }
}
