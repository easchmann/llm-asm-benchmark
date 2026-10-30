void ref_gesummv(long n, double alpha, double beta, const double *A, const double *B, double *tmp,
                 const double *x, double *y)
{
    for (long i = 0; i < n; i++) {
        tmp[i] = 0.0;
        y[i] = 0.0;
        for (long j = 0; j < n; j++) {
            tmp[i] = A[i * n + j] * x[j] + tmp[i];
            y[i] = B[i * n + j] * x[j] + y[i];
        }
        y[i] = alpha * (tmp[i] + y[i]);
    }
}
