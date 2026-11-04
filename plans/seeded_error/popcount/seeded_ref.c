void ref_popcount(long n, const unsigned long *a, long *counts)
{
    for (long i = 0; i < n; i++) {
        unsigned long v = a[i];
        v = v - ((v >> 1) & 0x5555555555555555UL);
        v = (v & 0x3333333333333333UL) + ((v >> 2) & 0x3333333333333333UL);
        v = (v + (v >> 4)) & 0x0F0F0F0F0F0F0F0FUL;
        counts[i] = (long)((v * 0x0101010101010101UL) & 0xFF);
    }
}
