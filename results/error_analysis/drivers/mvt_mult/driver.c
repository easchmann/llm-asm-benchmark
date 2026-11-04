#include "check.h"

void mvt(long n, double *x1, double *x2, const double *y_1, const double *y_2, const double *A);
void ref_mvt(long n, double *x1, double *x2, const double *y_1, const double *y_2, const double *A);

static const long SIZES[] = {8, 16, 64, 256};

int main(void)
{
    for (unsigned k = 0; k < sizeof SIZES / sizeof *SIZES; k++) {
        long n = SIZES[k];
        unsigned seed = 4000 + k;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        double *x1 = vec(n), *x2 = vec(n), *y_1 = vec(n), *y_2 = vec(n), *A = vec(n * n);
        fill(x1, n, &seed);
        fill(x2, n, &seed);
        fill(y_1, n, &seed);
        fill(y_2, n, &seed);
        fill(A, n * n, &seed);
        double *y_10 = copy(y_1, n), *y_20 = copy(y_2, n), *A0 = copy(A, n * n);
        double *x1_ref = copy(x1, n), *x2_ref = copy(x2, n);

        ref_mvt(n, x1_ref, x2_ref, y_10, y_20, A0);
        ABI_CALL(mvt, n, x1, x2, y_1, y_2, A);
        expect_abi();

        expect_close("x1", x1, x1_ref, n);
        expect_close("x2", x2, x2_ref, n);
        expect_unchanged("y_1", y_1, y_10, n);
        expect_unchanged("y_2", y_2, y_20, n);
        expect_unchanged("A", A, A0, n * n);
        expect_guards("x1", x1, n);
        expect_guards("x2", x2, n);
        expect_guards("y_1", y_1, n);
        expect_guards("y_2", y_2, n);
        expect_guards("A", A, n * n);
        end();

        vec_free(x1); vec_free(x2); vec_free(y_1); vec_free(y_2); vec_free(A);
        vec_free(y_10); vec_free(y_20); vec_free(A0); vec_free(x1_ref); vec_free(x2_ref);
    }
    return failed_sizes;
}
