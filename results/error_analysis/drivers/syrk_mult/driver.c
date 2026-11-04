#include "check.h"

void syrk(long n, long m, double alpha, double beta, double *C, const double *A);
void ref_syrk(long n, long m, double alpha, double beta, double *C, const double *A);

static const long SIZES[][2] = {{8, 8}, {16, 24}, {64, 32}, {256, 128}};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s][0], m = SIZES[s][1];
        double alpha = 1.5 + s, beta = -0.75 + 0.5 * s;
        unsigned seed = 9500 + s;
        char label[48];
        snprintf(label, sizeof label, "%ldx%ld", n, m);
        begin(label);

        double *C = vec(n * n), *A = vec(n * m);
        fill(C, n * n, &seed);
        fill(A, n * m, &seed);
        double *A0 = copy(A, n * m), *C_ref = copy(C, n * n);

        ref_syrk(n, m, alpha, beta, C_ref, A0);
        ABI_CALL(syrk, n, m, alpha, beta, C, A);
        expect_abi();

        expect_close_2d("C", C, C_ref, n, n);
        expect_unchanged("A", A, A0, n * m);
        expect_guards("C", C, n * n);
        expect_guards("A", A, n * m);
        end();

        vec_free(C); vec_free(A); vec_free(A0); vec_free(C_ref);
    }
    return failed_sizes;
}
