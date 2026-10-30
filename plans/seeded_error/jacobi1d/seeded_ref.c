void ref_jacobi1d(long tsteps, long n, double *A, double *B)
{
    for (long t = 0; t < tsteps; t++) {
        double *src = t % 2 ? B : A, *dst = t % 2 ? A : B;
        for (long i = 1; i < n - 1; i++)
            dst[i] = (src[i - 1] + src[i] + src[i + 1]) / 3.0;
    }
}
