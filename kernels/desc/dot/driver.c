#include "check.h"

double dot(long n, const double *x, const double *y);
double ref_dot(long n, const double *x, const double *y);

static const long SIZES[] = {1, 7, 33, 1001};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s];
        unsigned seed = 9000 + s;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        double *x = vec(n), *y = vec(n);
        fill(x, n, &seed);
        fill(y, n, &seed);
        double *x0 = copy(x, n), *y0 = copy(y, n);

        double want = ref_dot(n, x0, y0);
        double got = dot(n, x, y);

        expect_close("return value", &got, &want, 1);
        expect_unchanged("x", x, x0, n);
        expect_unchanged("y", y, y0, n);
        expect_guards("x", x, n);
        expect_guards("y", y, n);
        end();

        vec_free(x); vec_free(y); vec_free(x0); vec_free(y0);
    }
    return failed_sizes;
}
