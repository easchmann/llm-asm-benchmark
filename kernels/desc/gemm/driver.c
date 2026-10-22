#include "check.h"

void gemm(long ni, long nj, long nk, double alpha, double beta, double *C, const double *A, const double *B);
void ref_gemm(long ni, long nj, long nk, double alpha, double beta, double *C, const double *A, const double *B);

static const long SIZES[][3] = {{1, 1, 1}, {3, 5, 7}, {7, 5, 3}, {40, 50, 30}};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long ni = SIZES[s][0], nj = SIZES[s][1], nk = SIZES[s][2];
        double alpha = 1.5 + s, beta = -0.75 + 0.5 * s;
        unsigned seed = 5000 + s;
        char label[48];
        snprintf(label, sizeof label, "%ldx%ldx%ld", ni, nj, nk);
        begin(label);

        double *C = vec(ni * nj), *A = vec(ni * nk), *B = vec(nk * nj);
        fill(C, ni * nj, &seed);
        fill(A, ni * nk, &seed);
        fill(B, nk * nj, &seed);
        double *A0 = copy(A, ni * nk), *B0 = copy(B, nk * nj), *C_ref = copy(C, ni * nj);

        ref_gemm(ni, nj, nk, alpha, beta, C_ref, A0, B0);
        ABI_CALL(gemm, ni, nj, nk, alpha, beta, C, A, B);
        expect_abi();

        expect_close_2d("C", C, C_ref, ni, nj);
        expect_unchanged("A", A, A0, ni * nk);
        expect_unchanged("B", B, B0, nk * nj);
        expect_guards("C", C, ni * nj);
        expect_guards("A", A, ni * nk);
        expect_guards("B", B, nk * nj);
        end();

        vec_free(C); vec_free(A); vec_free(B);
        vec_free(A0); vec_free(B0); vec_free(C_ref);
    }
    return failed_sizes;
}
