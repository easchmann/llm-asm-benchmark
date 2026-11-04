#include "check.h"

void gemver(long n, double alpha, double beta, double *A, const double *u1, const double *v1,
            const double *u2, const double *v2, double *w, double *x, const double *y, const double *z);
void ref_gemver(long n, double alpha, double beta, double *A, const double *u1, const double *v1,
                const double *u2, const double *v2, double *w, double *x, const double *y, const double *z);

static const long SIZES[] = {8, 16, 64, 256};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s];
        double alpha = 1.5 + s, beta = -0.75 + 0.5 * s;
        unsigned seed = 6000 + s;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        double *A = vec(n * n), *u1 = vec(n), *v1 = vec(n), *u2 = vec(n), *v2 = vec(n);
        double *w = vec(n), *x = vec(n), *y = vec(n), *z = vec(n);
        fill(A, n * n, &seed);
        fill(u1, n, &seed);
        fill(v1, n, &seed);
        fill(u2, n, &seed);
        fill(v2, n, &seed);
        fill(w, n, &seed);
        fill(x, n, &seed);
        fill(y, n, &seed);
        fill(z, n, &seed);
        double *u10 = copy(u1, n), *v10 = copy(v1, n), *u20 = copy(u2, n), *v20 = copy(v2, n);
        double *y0 = copy(y, n), *z0 = copy(z, n);
        double *A_ref = copy(A, n * n), *w_ref = copy(w, n), *x_ref = copy(x, n);

        ref_gemver(n, alpha, beta, A_ref, u10, v10, u20, v20, w_ref, x_ref, y0, z0);
        ABI_CALL(gemver, n, alpha, beta, A, u1, v1, u2, v2, w, x, y, z);
        expect_abi();

        expect_close_2d("A", A, A_ref, n, n);
        expect_close("x", x, x_ref, n);
        expect_close("w", w, w_ref, n);
        expect_unchanged("u1", u1, u10, n);
        expect_unchanged("v1", v1, v10, n);
        expect_unchanged("u2", u2, u20, n);
        expect_unchanged("v2", v2, v20, n);
        expect_unchanged("y", y, y0, n);
        expect_unchanged("z", z, z0, n);
        expect_guards("A", A, n * n);
        expect_guards("u1", u1, n);
        expect_guards("v1", v1, n);
        expect_guards("u2", u2, n);
        expect_guards("v2", v2, n);
        expect_guards("w", w, n);
        expect_guards("x", x, n);
        expect_guards("y", y, n);
        expect_guards("z", z, n);
        end();

        vec_free(A); vec_free(u1); vec_free(v1); vec_free(u2); vec_free(v2);
        vec_free(w); vec_free(x); vec_free(y); vec_free(z);
        vec_free(u10); vec_free(v10); vec_free(u20); vec_free(v20); vec_free(y0); vec_free(z0);
        vec_free(A_ref); vec_free(w_ref); vec_free(x_ref);
    }
    return failed_sizes;
}
