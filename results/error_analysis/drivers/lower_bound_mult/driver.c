#include "check_int.h"

void lower_bound(long n, const long *sorted, long q, const long *keys, long *idx);
void ref_lower_bound(long n, const long *sorted, long q, const long *keys, long *idx);

static const long SIZES[][2] = {{8, 8}, {16, 24}, {64, 32}, {256, 128}};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s][0], q = SIZES[s][1];
        unsigned seed = 10400 + s;
        char label[48];
        snprintf(label, sizeof label, "%ldx%ld", n, q);
        begin(label);

        long *sorted = ivec(n), *keys = ivec(q), *idx = ivec(q);
        sorted[0] = -50;
        for (long i = 1; i < n; i++)
            sorted[i] = sorted[i - 1] + rand_r(&seed) % 4;
        ifill(keys, q, &seed, sorted[0] - 3, sorted[n - 1] + 3);
        keys[0] = sorted[0] - 1;
        if (q > 1)
            keys[1] = sorted[n - 1] + 1;
        if (q > 2)
            keys[2] = sorted[n / 2];
        ifill(idx, q, &seed, -1000, 1000);
        long *sorted0 = icopy(sorted, n), *keys0 = icopy(keys, q), *idx_ref = icopy(idx, q);

        ref_lower_bound(n, sorted0, q, keys0, idx_ref);
        ABI_CALL(lower_bound, n, sorted, q, keys, idx);
        expect_abi();

        expect_equal("idx", idx, idx_ref, q);
        iexpect_unchanged("sorted", sorted, sorted0, n);
        iexpect_unchanged("keys", keys, keys0, q);
        iexpect_guards("sorted", sorted, n);
        iexpect_guards("keys", keys, q);
        iexpect_guards("idx", idx, q);
        end();

        ivec_free(sorted); ivec_free(keys); ivec_free(idx);
        ivec_free(sorted0); ivec_free(keys0); ivec_free(idx_ref);
    }
    return failed_sizes;
}
