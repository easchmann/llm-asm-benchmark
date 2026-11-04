void ref_axpy(long n, double a, const double *x, double *y)
{
    for (long i = 0; i < n; i++)
        y[i] = a * y[i] + x[i];
}
