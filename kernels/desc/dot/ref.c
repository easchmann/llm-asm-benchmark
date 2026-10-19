double ref_dot(long n, const double *x, const double *y)
{
    double s = 0.0;
    for (long i = 0; i < n; i++)
        s += x[i] * y[i];
    return s;
}
