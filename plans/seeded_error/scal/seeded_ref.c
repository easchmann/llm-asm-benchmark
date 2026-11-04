void ref_scal(long n, double a, double *x)
{
    for (long i = 0; i < n; i++)
        x[i] = x[i] / a;
}
