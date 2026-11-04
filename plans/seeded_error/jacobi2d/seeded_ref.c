void ref_jacobi2d(long tsteps, long n, double *A, double *B)
{
    for (long t = 0; t < tsteps; t++) {
        for (long i = 1; i < n - 1; i++)
            for (long j = 1; j < n - 1; j++)
                B[i * n + j] = 0.25 * (A[i * n + j] + A[i * n + j - 1] + A[i * n + j + 1]
                                      + A[(i + 1) * n + j] + A[(i - 1) * n + j]);
        for (long i = 1; i < n - 1; i++)
            for (long j = 1; j < n - 1; j++)
                A[i * n + j] = 0.25 * (B[i * n + j] + B[i * n + j - 1] + B[i * n + j + 1]
                                      + B[(i + 1) * n + j] + B[(i - 1) * n + j]);
    }
}
