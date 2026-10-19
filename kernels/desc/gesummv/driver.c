#include "check.h"

void gesummv(long n, double alpha, double beta, const double *A, const double *B, double *tmp,
             const double *x, double *y);
void ref_gesummv(long n, double alpha, double beta, const double *A, const double *B, double *tmp,
                 const double *x, double *y);

static const long SIZES[] = {1, 7, 13, 90};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s];
        double alpha = 1.5 + s, beta = -0.75 + 0.5 * s;
        unsigned seed = 9300 + s;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        double *A = vec(n * n), *B = vec(n * n), *tmp = vec(n), *x = vec(n), *y = vec(n);
        fill(A, n * n, &seed);
        fill(B, n * n, &seed);
        fill(tmp, n, &seed);
        fill(x, n, &seed);
        fill(y, n, &seed);
        double *A0 = copy(A, n * n), *B0 = copy(B, n * n), *x0 = copy(x, n);
        double *tmp_ref = copy(tmp, n), *y_ref = copy(y, n);

        ref_gesummv(n, alpha, beta, A0, B0, tmp_ref, x0, y_ref);
        gesummv(n, alpha, beta, A, B, tmp, x, y);

        expect_close("y", y, y_ref, n);
        expect_unchanged("A", A, A0, n * n);
        expect_unchanged("B", B, B0, n * n);
        expect_unchanged("x", x, x0, n);
        expect_guards("A", A, n * n);
        expect_guards("B", B, n * n);
        expect_guards("tmp", tmp, n);
        expect_guards("x", x, n);
        expect_guards("y", y, n);
        end();

        vec_free(A); vec_free(B); vec_free(tmp); vec_free(x); vec_free(y);
        vec_free(A0); vec_free(B0); vec_free(x0); vec_free(tmp_ref); vec_free(y_ref);
    }
    return failed_sizes;
}
