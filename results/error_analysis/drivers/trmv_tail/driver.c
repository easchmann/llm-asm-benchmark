#include "check.h"

void trmv(long n, const double *L, const double *x, double *y);
void ref_trmv(long n, const double *L, const double *x, double *y);

static const long SIZES[] = {1, 3, 6, 7, 13, 67, 255};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s];
        unsigned seed = 9400 + s;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        double *L = vec(n * n), *x = vec(n), *y = vec(n);
        fill(L, n * n, &seed);
        fill(x, n, &seed);
        fill(y, n, &seed);
        double *L0 = copy(L, n * n), *x0 = copy(x, n), *y_ref = copy(y, n);

        ref_trmv(n, L0, x0, y_ref);
        ABI_CALL(trmv, n, L, x, y);
        expect_abi();

        expect_close("y", y, y_ref, n);
        expect_unchanged("L", L, L0, n * n);
        expect_unchanged("x", x, x0, n);
        expect_guards("L", L, n * n);
        expect_guards("x", x, n);
        expect_guards("y", y, n);
        end();

        vec_free(L); vec_free(x); vec_free(y); vec_free(L0); vec_free(x0); vec_free(y_ref);
    }
    return failed_sizes;
}
