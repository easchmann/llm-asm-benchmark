void ref_spmv(long nrows, long ncols, const long *rowptr, const long *col, const double *val,
              const double *x, double *y)
{
    (void)ncols;
    for (long i = 0; i < nrows; i++) {
        double s = 0.0;
        for (long k = rowptr[i]; k < rowptr[i + 1]; k++)
            s += val[k] * x[col[k]];
        y[i] = s;
    }
}
