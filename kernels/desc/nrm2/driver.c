#include "check.h"

double nrm2(long n, const double *x);
double ref_nrm2(long n, const double *x);

static const long SIZES[] = {1, 7, 33, 1001};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s];
        unsigned seed = 9200 + s;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        double *x = vec(n);
        fill(x, n, &seed);
        double *x0 = copy(x, n);

        double want = ref_nrm2(n, x0);
        double got = nrm2(n, x);

        expect_close("return value", &got, &want, 1);
        expect_unchanged("x", x, x0, n);
        expect_guards("x", x, n);
        end();

        vec_free(x); vec_free(x0);
    }
    return failed_sizes;
}
