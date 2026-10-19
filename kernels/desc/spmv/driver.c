#include "check_int.h"

void spmv(long nrows, long ncols, const long *rowptr, const long *col, const double *val,
          const double *x, double *y);
void ref_spmv(long nrows, long ncols, const long *rowptr, const long *col, const double *val,
              const double *x, double *y);

static const long SIZES[][2] = {{1, 1}, {5, 7}, {7, 5}, {200, 150}};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long nrows = SIZES[s][0], ncols = SIZES[s][1];
        unsigned seed = 10300 + s;
        char label[48];
        snprintf(label, sizeof label, "%ldx%ld", nrows, ncols);
        begin(label);

        long *rowptr = ivec(nrows + 1);
        rowptr[0] = 0;
        for (long i = 0; i < nrows; i++) {
            long per_row = rand_r(&seed) % 9;
            if (nrows > 1 && i == 1)
                per_row = 0;
            rowptr[i + 1] = rowptr[i] + per_row;
        }
        long nnz = rowptr[nrows] > 0 ? rowptr[nrows] : 1;
        long *col = ivec(nnz);
        double *val = vec(nnz), *x = vec(ncols), *y = vec(nrows);
        ifill(col, nnz, &seed, 0, ncols - 1);
        fill(val, nnz, &seed);
        fill(x, ncols, &seed);
        fill(y, nrows, &seed);
        long *rowptr0 = icopy(rowptr, nrows + 1), *col0 = icopy(col, nnz);
        double *val0 = copy(val, nnz), *x0 = copy(x, ncols), *y_ref = copy(y, nrows);

        ref_spmv(nrows, ncols, rowptr0, col0, val0, x0, y_ref);
        spmv(nrows, ncols, rowptr, col, val, x, y);

        expect_close("y", y, y_ref, nrows);
        iexpect_unchanged("rowptr", rowptr, rowptr0, nrows + 1);
        iexpect_unchanged("col", col, col0, nnz);
        expect_unchanged("val", val, val0, nnz);
        expect_unchanged("x", x, x0, ncols);
        iexpect_guards("rowptr", rowptr, nrows + 1);
        iexpect_guards("col", col, nnz);
        expect_guards("val", val, nnz);
        expect_guards("x", x, ncols);
        expect_guards("y", y, nrows);
        end();

        ivec_free(rowptr); ivec_free(col); vec_free(val); vec_free(x); vec_free(y);
        ivec_free(rowptr0); ivec_free(col0); vec_free(val0); vec_free(x0); vec_free(y_ref);
    }
    return failed_sizes;
}
