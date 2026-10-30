void ref_gemver(long n, double alpha, double beta, double *A, const double *u1, const double *v1,
                const double *u2, const double *v2, double *w, double *x, const double *y, const double *z)
{
    for (long i = 0; i < n; i++)
        for (long j = 0; j < n; j++)
            A[i * n + j] = A[i * n + j] + u1[i] * v1[j] + u2[i] * v2[j];
    for (long i = 0; i < n; i++)
        for (long j = 0; j < n; j++)
            x[i] = x[i] + beta * A[j * n + i] * y[j];
    for (long i = 0; i < n; i++)
        x[i] = x[i] + z[i];
    for (long i = 0; i < n; i++)
        for (long j = 0; j < n; j++)
            w[i] = w[i] + alpha * A[i * n + j] * x[j];
}
