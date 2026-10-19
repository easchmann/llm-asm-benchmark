#include "check_int.h"

void prefix_sum(long n, const long *a, long *out);
void ref_prefix_sum(long n, const long *a, long *out);

static const long SIZES[] = {1, 7, 33, 1001};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s];
        unsigned seed = 10000 + s;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        long *a = ivec(n), *out = ivec(n);
        ifill(a, n, &seed, -1000, 1000);
        ifill(out, n, &seed, -1000, 1000);
        long *a0 = icopy(a, n), *out_ref = icopy(out, n);

        ref_prefix_sum(n, a0, out_ref);
        ABI_CALL(prefix_sum, n, a, out);
        expect_abi();

        expect_equal("out", out, out_ref, n);
        iexpect_unchanged("a", a, a0, n);
        iexpect_guards("a", a, n);
        iexpect_guards("out", out, n);
        end();

        ivec_free(a); ivec_free(out); ivec_free(a0); ivec_free(out_ref);
    }
    return failed_sizes;
}
