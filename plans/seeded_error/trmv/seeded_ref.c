void ref_trmv(long n, const double *L, const double *x, double *y)
{
    for (long i = 0; i < n; i++) {
        y[i] = 0.0;
        for (long j = 0; j < i; j++)
            y[i] += L[i * n + j] * x[j];
    }
}
