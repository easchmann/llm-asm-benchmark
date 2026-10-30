#include "bench.h"

void spmv(long nrows, long ncols, const long *rowptr, const long *col, const double *val,
          const double *x, double *y);
void ref_spmv(long nrows, long ncols, const long *rowptr, const long *col, const double *val,
              const double *x, double *y);

static long nrows, ncols;
static long *rowptr, *col;
static double *val, *x, *y;

static void setup(int large)
{
    nrows = large ? 2000000 : 2000;
    ncols = large ? 2500000 : 2400;
    rowptr = larr("rowptr", nrows + 1, IN);
    rowptr[0] = 0;
    for (long i = 0; i < nrows; i++)
        rowptr[i + 1] = rowptr[i] + uniform(0, 8);
    long nnz = rowptr[nrows];
    col = larr("col", nnz, IN);
    for (long k = 0; k < nnz; k++)
        col[k] = uniform(0, ncols - 1);
    val = darr("val", nnz, IN);
    x = darr("x", ncols, IN);
    y = darr("y", nrows, OUT);
}

static void call(int ref)
{
    if (ref)
        ref_spmv(nrows, ncols, rowptr, col, val, x, y);
    else
        CALL(spmv, nrows, ncols, rowptr, col, val, x, y);
}
