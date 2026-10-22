#ifndef DESC_CHECK_INT_H
#define DESC_CHECK_INT_H

#include "check.h"

_Static_assert(sizeof(long) == sizeof(double), "long and double must both be 8 bytes");

static long *ivec(long n)
{
    return (long *)vec(n);
}

static void ivec_free(long *v)
{
    vec_free((double *)v);
}

static long *icopy(const long *v, long n)
{
    return (long *)copy((const double *)v, n);
}

static void ifill(long *v, long n, unsigned *seed, long lo, long hi)
{
    for (long i = 0; i < n; i++)
        v[i] = lo + rand_r(seed) % (hi - lo + 1);
}

static unsigned long random_u64(unsigned *seed)
{
    unsigned long a = rand_r(seed), b = rand_r(seed), c = rand_r(seed);
    return (a << 33) ^ (b << 2) ^ c;
}

static void expect_equal(const char *name, const long *got, const long *want, long n)
{
    long wrong = 0, first = -1;
    for (long i = 0; i < n; i++) {
        if (got[i] != want[i]) {
            wrong++;
            if (first < 0)
                first = i;
        }
    }
#ifdef DESC_DIAG
    char at[96];
    if (wrong) {
        where(at, sizeof at, name, first);
        fail("%s: %ld/%ld wrong, first wrong element %s", name, wrong, n, at);
    }
#else
    if (wrong)
        fail("%s: %ld/%ld wrong, first at %ld: got %ld, want %ld",
             name, wrong, n, first, got[first], want[first]);
#endif
}

static void iexpect_unchanged(const char *name, const long *now, const long *before, long n)
{
    expect_unchanged(name, (const double *)now, (const double *)before, n);
}

static void iexpect_guards(const char *name, const long *v, long n)
{
    expect_guards(name, (const double *)v, n);
}

#endif
