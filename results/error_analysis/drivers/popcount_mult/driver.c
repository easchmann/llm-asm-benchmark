#include "check_int.h"

void popcount(long n, const unsigned long *a, long *counts);
void ref_popcount(long n, const unsigned long *a, long *counts);

static const long SIZES[] = {8, 16, 64, 1024};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s];
        unsigned seed = 10200 + s;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        long *a = ivec(n), *counts = ivec(n);
        for (long i = 0; i < n; i++)
            a[i] = (long)random_u64(&seed);
        if (n >= 2) {
            a[0] = 0;
            a[1] = -1;
        }
        ifill(counts, n, &seed, -1000, 1000);
        long *a0 = icopy(a, n), *counts_ref = icopy(counts, n);

        ref_popcount(n, (const unsigned long *)a0, counts_ref);
        ABI_CALL(popcount, n, (const unsigned long *)a, counts);
        expect_abi();

        expect_equal("counts", counts, counts_ref, n);
        iexpect_unchanged("a", a, a0, n);
        iexpect_guards("a", a, n);
        iexpect_guards("counts", counts, n);
        end();

        ivec_free(a); ivec_free(counts); ivec_free(a0); ivec_free(counts_ref);
    }
    return failed_sizes;
}
