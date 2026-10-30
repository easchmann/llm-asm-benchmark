void ref_jacobi1d(long tsteps, long n, double *A, double *B)
{
    for (long t = 0; t < tsteps; t++) {
        for (long i = 1; i < n - 1; i++)
            B[i] = (A[i - 1] + A[i] + A[i + 1]) / 3.0;
        for (long i = 1; i < n - 1; i++)
            A[i] = (B[i - 1] + B[i] + B[i + 1]) / 3.0;
    }
}
