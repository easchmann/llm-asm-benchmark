void ref_mvt(long n, double *x1, double *x2, const double *y_1, const double *y_2, const double *A)
{
    for (long i = 0; i < n; i++)
        for (long j = 0; j < n; j++)
            x1[i] = x1[i] + A[i * n + j] * y_1[j];
    for (long i = 0; i < n; i++)
        for (long j = 0; j < n; j++)
            x2[i] = x2[i] + A[j * n + i] * y_2[j];
}
