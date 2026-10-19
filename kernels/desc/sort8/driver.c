#include "check_int.h"

void sort8(long n, long *a);
void ref_sort8(long n, long *a);

static const long SIZES[] = {8, 16, 64, 800};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s];
        unsigned seed = 10500 + s;
        char label[32];
        snprintf(label, sizeof label, "%ld", n);
        begin(label);

        long *a = ivec(n);
        ifill(a, n, &seed, -20, 20);
        if (n >= 16)
            for (long i = 0; i < 8; i++)
                a[8 + i] = 7 - i;
        long *a_ref = icopy(a, n);

        ref_sort8(n, a_ref);
        ABI_CALL(sort8, n, a);
        expect_abi();

        expect_equal("a", a, a_ref, n);
        iexpect_guards("a", a, n);
        end();

        ivec_free(a); ivec_free(a_ref);
    }
    return failed_sizes;
}
