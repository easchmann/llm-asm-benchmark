#ifndef DESC_CHECK_H
#define DESC_CHECK_H

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RTOL 1e-9
#define ATOL 1e-12
#define GUARD 64
#define GUARD_BYTE 0xA5

static const char *current_size;
static int size_failures;
static int failed_sizes;

static double *vec(long n)
{
    size_t bytes = 2 * GUARD + n * sizeof(double);
    unsigned char *p = aligned_alloc(GUARD, (bytes + GUARD - 1) / GUARD * GUARD);
    memset(p, GUARD_BYTE, GUARD);
    memset(p + GUARD + n * sizeof(double), GUARD_BYTE, GUARD);
    return (double *)(p + GUARD);
}

static void vec_free(double *v)
{
    free((unsigned char *)v - GUARD);
}

static void fill(double *v, long n, unsigned *seed)
{
    for (long i = 0; i < n; i++)
        v[i] = (double)(rand_r(seed) % 2001 - 1000) / 100.0;
}

static double *copy(const double *v, long n)
{
    double *c = vec(n);
    memcpy(c, v, n * sizeof(double));
    return c;
}

static void begin(const char *label)
{
    current_size = label;
    size_failures = 0;
}

static void fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("size %s: fail ", current_size);
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
    size_failures++;
}

static void end(void)
{
    if (size_failures)
        failed_sizes++;
    else
        printf("size %s: pass\n", current_size);
    fflush(stdout);
}

static void expect_close(const char *name, const double *got, const double *want, long n)
{
    long wrong = 0;
    double worst = 0;
    for (long i = 0; i < n; i++) {
        double err = fabs(got[i] - want[i]);
        if (!(err <= RTOL * fabs(want[i]) + ATOL)) {
            wrong++;
            if (!(err <= worst))
                worst = err;
        }
    }
    if (wrong)
        fail("%s: %ld/%ld wrong, max abs err %g", name, wrong, n, worst);
}

static void expect_unchanged(const char *name, const double *now, const double *before, long n)
{
    if (memcmp(now, before, n * sizeof(double)))
        fail("%s: input was modified", name);
}

static void expect_guards(const char *name, const double *v, long n)
{
    const unsigned char *lo = (const unsigned char *)v - GUARD;
    const unsigned char *hi = (const unsigned char *)(v + n);
    for (int i = 0; i < GUARD; i++) {
        if (lo[i] != GUARD_BYTE || hi[i] != GUARD_BYTE) {
            fail("%s: write outside the array", name);
            return;
        }
    }
}

#endif
