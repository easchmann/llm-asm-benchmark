#include "check.h"

void mm2(long ni, long nj, long nk, long nl, double alpha, double beta, double *tmp,
         const double *A, const double *B, const double *C, double *D);
void ref_2mm(long ni, long nj, long nk, long nl, double alpha, double beta, double *tmp,
             const double *A, const double *B, const double *C, double *D);

static const long SIZES[][4] = {{1, 1, 1, 1}, {3, 5, 7, 4}, {7, 4, 3, 5}, {30, 40, 50, 20}};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long ni = SIZES[s][0], nj = SIZES[s][1], nk = SIZES[s][2], nl = SIZES[s][3];
        double alpha = 1.5 + s, beta = -0.75 + 0.5 * s;
        unsigned seed = 7000 + s;
        char label[64];
        snprintf(label, sizeof label, "%ldx%ldx%ldx%ld", ni, nj, nk, nl);
        begin(label);

        double *tmp = vec(ni * nj), *A = vec(ni * nk), *B = vec(nk * nj), *C = vec(nj * nl), *D = vec(ni * nl);
        fill(tmp, ni * nj, &seed);
        fill(A, ni * nk, &seed);
        fill(B, nk * nj, &seed);
        fill(C, nj * nl, &seed);
        fill(D, ni * nl, &seed);
        double *A0 = copy(A, ni * nk), *B0 = copy(B, nk * nj), *C0 = copy(C, nj * nl);
        double *tmp_ref = copy(tmp, ni * nj), *D_ref = copy(D, ni * nl);

        ref_2mm(ni, nj, nk, nl, alpha, beta, tmp_ref, A0, B0, C0, D_ref);
        ABI_CALL(mm2, ni, nj, nk, nl, alpha, beta, tmp, A, B, C, D);
        expect_abi();

        expect_close_2d("D", D, D_ref, ni, nl);
        expect_unchanged("A", A, A0, ni * nk);
        expect_unchanged("B", B, B0, nk * nj);
        expect_unchanged("C", C, C0, nj * nl);
        expect_guards("tmp", tmp, ni * nj);
        expect_guards("A", A, ni * nk);
        expect_guards("B", B, nk * nj);
        expect_guards("C", C, nj * nl);
        expect_guards("D", D, ni * nl);
        end();

        vec_free(tmp); vec_free(A); vec_free(B); vec_free(C); vec_free(D);
        vec_free(A0); vec_free(B0); vec_free(C0); vec_free(tmp_ref); vec_free(D_ref);
    }
    return failed_sizes;
}
