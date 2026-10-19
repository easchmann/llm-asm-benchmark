#include "check.h"

void trmm(long m, long n, double alpha, const double *A, double *B);
void ref_trmm(long m, long n, double alpha, const double *A, double *B);

static const long SIZES[][2] = {{1, 1}, {5, 7}, {7, 5}, {40, 30}};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long m = SIZES[s][0], n = SIZES[s][1];
        double alpha = 1.5 + s;
        unsigned seed = 9600 + s;
        char label[48];
        snprintf(label, sizeof label, "%ldx%ld", m, n);
        begin(label);

        double *A = vec(m * m), *B = vec(m * n);
        fill(A, m * m, &seed);
        fill(B, m * n, &seed);
        double *A0 = copy(A, m * m), *B_ref = copy(B, m * n);

        ref_trmm(m, n, alpha, A0, B_ref);
        ABI_CALL(trmm, m, n, alpha, A, B);
        expect_abi();

        expect_close("B", B, B_ref, m * n);
        expect_unchanged("A", A, A0, m * m);
        expect_guards("A", A, m * m);
        expect_guards("B", B, m * n);
        end();

        vec_free(A); vec_free(B); vec_free(A0); vec_free(B_ref);
    }
    return failed_sizes;
}
