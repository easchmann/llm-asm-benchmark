#include "check.h"

void heat3d(long tsteps, long n, double *A, double *B);
void ref_heat3d(long tsteps, long n, double *A, double *B);

static const long SIZES[][2] = {{1, 3}, {2, 4}, {3, 9}, {2, 20}};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long tsteps = SIZES[s][0], n = SIZES[s][1];
        long cells = n * n * n;
        unsigned seed = 9900 + s;
        char label[48];
        snprintf(label, sizeof label, "t%ld_n%ld", tsteps, n);
        begin(label);

        double *A = vec(cells), *B = vec(cells);
        fill(A, cells, &seed);
        fill(B, cells, &seed);
        double *A_ref = copy(A, cells), *B_ref = copy(B, cells);

        ref_heat3d(tsteps, n, A_ref, B_ref);
        ABI_CALL(heat3d, tsteps, n, A, B);
        expect_abi();

        expect_close_3d("A", A, A_ref, n, n, n);
        expect_close_3d("B", B, B_ref, n, n, n);
        expect_guards("A", A, cells);
        expect_guards("B", B, cells);
        end();

        vec_free(A); vec_free(B); vec_free(A_ref); vec_free(B_ref);
    }
    return failed_sizes;
}
