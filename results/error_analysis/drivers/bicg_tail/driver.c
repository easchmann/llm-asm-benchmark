#include "check.h"

void bicg(long m, long n, const double *A, double *s, double *q, const double *p, const double *r);
void ref_bicg(long m, long n, const double *A, double *s, double *q, const double *p, const double *r);

static const long SIZES[][2] = {{1, 1}, {3, 5}, {6, 7}, {13, 7}, {67, 35}, {255, 129}};

int main(void)
{
    for (unsigned k = 0; k < sizeof SIZES / sizeof *SIZES; k++) {
        long m = SIZES[k][0], n = SIZES[k][1];
        unsigned seed = 3000 + k;
        char label[32];
        snprintf(label, sizeof label, "%ldx%ld", m, n);
        begin(label);

        double *A = vec(n * m), *s = vec(m), *q = vec(n), *p = vec(m), *r = vec(n);
        fill(A, n * m, &seed);
        fill(s, m, &seed);
        fill(q, n, &seed);
        fill(p, m, &seed);
        fill(r, n, &seed);
        double *A0 = copy(A, n * m), *p0 = copy(p, m), *r0 = copy(r, n);
        double *s_ref = copy(s, m), *q_ref = copy(q, n);

        ref_bicg(m, n, A0, s_ref, q_ref, p0, r0);
        ABI_CALL(bicg, m, n, A, s, q, p, r);
        expect_abi();

        expect_close("s", s, s_ref, m);
        expect_close("q", q, q_ref, n);
        expect_unchanged("A", A, A0, n * m);
        expect_unchanged("p", p, p0, m);
        expect_unchanged("r", r, r0, n);
        expect_guards("A", A, n * m);
        expect_guards("s", s, m);
        expect_guards("q", q, n);
        expect_guards("p", p, m);
        expect_guards("r", r, n);
        end();

        vec_free(A); vec_free(s); vec_free(q); vec_free(p); vec_free(r);
        vec_free(A0); vec_free(p0); vec_free(r0); vec_free(s_ref); vec_free(q_ref);
    }
    return failed_sizes;
}
