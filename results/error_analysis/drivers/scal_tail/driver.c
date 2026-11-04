#include "check.h"

void scal(long n, double a, double *x);
void ref_scal(long n, double a, double *x);

static const long SIZES[] = {1, 3, 6, 7, 13, 67, 1003};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s];
        double a = -1.25 + s;
        unsigned seed = 9100 + s;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        double *x = vec(n);
        fill(x, n, &seed);
        double *x_ref = copy(x, n);

        ref_scal(n, a, x_ref);
        ABI_CALL(scal, n, a, x);
        expect_abi();

        expect_close("x", x, x_ref, n);
        expect_guards("x", x, n);
        end();

        vec_free(x); vec_free(x_ref);
    }
    return failed_sizes;
}
