void ref_trmm(long m, long n, double alpha, const double *A, double *B)
{
    for (long i = 0; i < m; i++)
        for (long j = 0; j < n; j++) {
            for (long k = i + 1; k < m; k++)
                B[i * n + j] += A[i * m + k] * B[k * n + j];
            B[i * n + j] = alpha * B[i * n + j];
        }
}
