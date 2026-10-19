#include "check.h"

void atax(long m, long n, const double *A, const double *x, double *y, double *tmp);
void ref_atax(long m, long n, const double *A, const double *x, double *y, double *tmp);

static const long SIZES[][2] = {{1, 1}, {7, 13}, {38, 42}, {300, 517}};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long m = SIZES[s][0], n = SIZES[s][1];
        unsigned seed = 1000 + s;
        char label[32];
        snprintf(label, sizeof label, "%ldx%ld", m, n);
        begin(label);

        double *A = vec(m * n), *x = vec(n), *y = vec(n), *tmp = vec(m);
        fill(A, m * n, &seed);
        fill(x, n, &seed);
        fill(y, n, &seed);
        fill(tmp, m, &seed);
        double *A0 = copy(A, m * n), *x0 = copy(x, n);
        double *y_ref = copy(y, n), *tmp_ref = copy(tmp, m);

        ref_atax(m, n, A0, x0, y_ref, tmp_ref);
        ABI_CALL(atax, m, n, A, x, y, tmp);
        expect_abi();

        expect_close("y", y, y_ref, n);
        expect_unchanged("A", A, A0, m * n);
        expect_unchanged("x", x, x0, n);
        expect_guards("A", A, m * n);
        expect_guards("x", x, n);
        expect_guards("y", y, n);
        expect_guards("tmp", tmp, m);
        end();

        vec_free(A); vec_free(x); vec_free(y); vec_free(tmp);
        vec_free(A0); vec_free(x0); vec_free(y_ref); vec_free(tmp_ref);
    }
    return failed_sizes;
}
