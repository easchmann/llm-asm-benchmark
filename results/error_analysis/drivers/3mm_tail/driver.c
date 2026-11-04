#include "check.h"

void mm3(long ni, long nj, long nk, long nl, long nm, double *E, const double *A, const double *B,
         double *F, const double *C, const double *D, double *G);
void ref_3mm(long ni, long nj, long nk, long nl, long nm, double *E, const double *A, const double *B,
             double *F, const double *C, const double *D, double *G);

static const long SIZES[][5] = {{1, 1, 1, 1, 1}, {3, 5, 7, 6, 4}, {7, 6, 5, 3, 9}, {13, 7, 9, 11, 5}, {33, 41, 47, 21, 35}};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long ni = SIZES[s][0], nj = SIZES[s][1], nk = SIZES[s][2], nl = SIZES[s][3], nm = SIZES[s][4];
        unsigned seed = 8000 + s;
        char label[80];
        snprintf(label, sizeof label, "%ldx%ldx%ldx%ldx%ld", ni, nj, nk, nl, nm);
        begin(label);

        double *E = vec(ni * nj), *A = vec(ni * nk), *B = vec(nk * nj), *F = vec(nj * nl);
        double *C = vec(nj * nm), *D = vec(nm * nl), *G = vec(ni * nl);
        fill(E, ni * nj, &seed);
        fill(A, ni * nk, &seed);
        fill(B, nk * nj, &seed);
        fill(F, nj * nl, &seed);
        fill(C, nj * nm, &seed);
        fill(D, nm * nl, &seed);
        fill(G, ni * nl, &seed);
        double *A0 = copy(A, ni * nk), *B0 = copy(B, nk * nj), *C0 = copy(C, nj * nm), *D0 = copy(D, nm * nl);
        double *E_ref = copy(E, ni * nj), *F_ref = copy(F, nj * nl), *G_ref = copy(G, ni * nl);

        ref_3mm(ni, nj, nk, nl, nm, E_ref, A0, B0, F_ref, C0, D0, G_ref);
        ABI_CALL(mm3, ni, nj, nk, nl, nm, E, A, B, F, C, D, G);
        expect_abi();

        expect_close_2d("G", G, G_ref, ni, nl);
        expect_unchanged("A", A, A0, ni * nk);
        expect_unchanged("B", B, B0, nk * nj);
        expect_unchanged("C", C, C0, nj * nm);
        expect_unchanged("D", D, D0, nm * nl);
        expect_guards("E", E, ni * nj);
        expect_guards("A", A, ni * nk);
        expect_guards("B", B, nk * nj);
        expect_guards("F", F, nj * nl);
        expect_guards("C", C, nj * nm);
        expect_guards("D", D, nm * nl);
        expect_guards("G", G, ni * nl);
        end();

        vec_free(E); vec_free(A); vec_free(B); vec_free(F); vec_free(C); vec_free(D); vec_free(G);
        vec_free(A0); vec_free(B0); vec_free(C0); vec_free(D0); vec_free(E_ref); vec_free(F_ref); vec_free(G_ref);
    }
    return failed_sizes;
}
