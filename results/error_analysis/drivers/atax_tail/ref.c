void ref_atax(long m, long n, const double *A, const double *x, double *y, double *tmp)
{
    for (long j = 0; j < n; j++)
        y[j] = 0.0;
    for (long i = 0; i < m; i++) {
        tmp[i] = 0.0;
        for (long j = 0; j < n; j++)
            tmp[i] += A[i * n + j] * x[j];
        for (long j = 0; j < n; j++)
            y[j] += A[i * n + j] * tmp[i];
    }
}
