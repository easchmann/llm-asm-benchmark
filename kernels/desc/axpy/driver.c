#include "check.h"

void axpy(long n, double a, const double *x, double *y);
void ref_axpy(long n, double a, const double *x, double *y);

static const long SIZES[] = {1, 7, 42, 1001};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s];
        double a = 1.5 + s;
        unsigned seed = 2000 + s;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        double *x = vec(n), *y = vec(n);
        fill(x, n, &seed);
        fill(y, n, &seed);
        double *x0 = copy(x, n), *y_ref = copy(y, n);

        ref_axpy(n, a, x0, y_ref);
        axpy(n, a, x, y);

        expect_close("y", y, y_ref, n);
        expect_unchanged("x", x, x0, n);
        expect_guards("x", x, n);
        expect_guards("y", y, n);
        end();

        vec_free(x); vec_free(y); vec_free(x0); vec_free(y_ref);
    }
    return failed_sizes;
}
