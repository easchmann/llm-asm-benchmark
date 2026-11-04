#include "check_int.h"

void histogram(long n, const long *data, long nbins, long *hist);
void ref_histogram(long n, const long *data, long nbins, long *hist);

static const long SIZES[][2] = {{1, 1}, {3, 5}, {6, 7}, {13, 7}, {67, 35}, {255, 129}};

int main(void)
{
    for (unsigned s = 0; s < sizeof SIZES / sizeof *SIZES; s++) {
        long n = SIZES[s][0], nbins = SIZES[s][1];
        unsigned seed = 10100 + s;
        char label[48];
        snprintf(label, sizeof label, "%ldx%ld", n, nbins);
        begin(label);

        long *data = ivec(n), *hist = ivec(nbins);
        ifill(data, n, &seed, 0, nbins - 1);
        ifill(hist, nbins, &seed, -1000, 1000);
        long *data0 = icopy(data, n), *hist_ref = icopy(hist, nbins);

        ref_histogram(n, data0, nbins, hist_ref);
        ABI_CALL(histogram, n, data, nbins, hist);
        expect_abi();

        expect_equal("hist", hist, hist_ref, nbins);
        iexpect_unchanged("data", data, data0, n);
        iexpect_guards("data", data, n);
        iexpect_guards("hist", hist, nbins);
        end();

        ivec_free(data); ivec_free(hist); ivec_free(data0); ivec_free(hist_ref);
    }
    return failed_sizes;
}
