void ref_syrk(long n, long m, double alpha, double beta, double *C, const double *A)
{
    for (long i = 0; i < n; i++) {
        for (long j = 0; j <= i; j++)
            C[i * n + j] *= beta;
        for (long k = 0; k < m; k++)
            for (long j = 0; j <= i; j++)
                C[i * n + j] += alpha * A[i * m + k] * A[j * m + k];
    }
}
