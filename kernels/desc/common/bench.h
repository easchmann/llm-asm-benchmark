#ifndef DESC_BENCH_H
#define DESC_BENCH_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <linux/perf_event.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "check.h"

#define BENCH_RTOL 1e-8
#define HUGE_PAGE (2UL << 20)
#define MAX_ARRAYS 16
#define MAX_SAMPLES 300
#define MIN_SAMPLES 21
#define MIN_LONG_SAMPLES 5
#define LONG_SAMPLE_NS 5e7
#define MIN_TIMED_NS 3e8
#define BATCH_NS 5e4
#define NCOUNTERS 5

enum { IN, INOUT, OUT, SCRATCH };

static struct {
    const char *name;
    void *data, *pristine, *want;
    size_t bytes;
    int kind, is_long;
} arrays[MAX_ARRAYS];
static int narrays;
static int batchable = 1;
static int checking;
static unsigned long rng = 0x9E3779B97F4A7C15UL;

static void setup(int large);
static void call(int ref);

#define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))

static unsigned long next(void)
{
    unsigned long z = (rng += 0x9E3779B97F4A7C15UL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9UL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBUL;
    return z ^ (z >> 31);
}

static long uniform(long lo, long hi)
{
    return lo + (long)(next() % (unsigned long)(hi - lo + 1));
}

static void *place(const char *name, size_t bytes, int kind, int is_long)
{
    size_t offset = GUARD + narrays * 5 * GUARD, total = offset + bytes + GUARD;
    unsigned char *p = mmap(0, total + HUGE_PAGE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED || narrays == MAX_ARRAYS) {
        printf("{\"status\": \"setup_fail\"}\n");
        exit(2);
    }
    unsigned char *base = (unsigned char *)(((uintptr_t)p + HUGE_PAGE - 1) & ~(HUGE_PAGE - 1));
    madvise(base, total, MADV_HUGEPAGE);
    unsigned char *a = base + offset;
    memset(a - GUARD, GUARD_BYTE, GUARD);
    memset(a + bytes, GUARD_BYTE, GUARD);
    arrays[narrays].name = name;
    arrays[narrays].data = a;
    arrays[narrays].bytes = bytes;
    arrays[narrays].kind = kind;
    arrays[narrays].is_long = is_long;
    narrays++;
    return a;
}

static double *darr(const char *name, long n, int kind)
{
    double *v = place(name, n * sizeof(double), kind, 0);
    for (long i = 0; i < n; i++)
        v[i] = (double)uniform(-1000, 1000) / 100.0;
    return v;
}

static long *larr(const char *name, long n, int kind)
{
    long *v = place(name, n * sizeof(long), kind, 1);
    for (long i = 0; i < n; i++)
        v[i] = uniform(-1000, 1000);
    return v;
}

static void restore(void)
{
    for (int i = 0; i < narrays; i++)
        if (arrays[i].kind != IN)
            memcpy(arrays[i].data, arrays[i].pristine, arrays[i].bytes);
}

static double normwise_error(const double *got, const double *want, long n)
{
    double err = 0, scale = 0;
    for (long i = 0; i < n; i++) {
        double e = fabs(got[i] - want[i]);
        if (!(e <= err))
            err = e;
        if (fabs(want[i]) > scale)
            scale = fabs(want[i]);
    }
    return scale > 0 ? err / scale : err;
}

static double verify(void)
{
    double worst = 0;
    expect_abi();
    for (int i = 0; i < narrays; i++) {
        const char *name = arrays[i].name;
        long n = arrays[i].bytes / 8;
        current_size = name;
        const unsigned char *lo = (const unsigned char *)arrays[i].data - GUARD, *hi = lo + GUARD + arrays[i].bytes;
        for (int g = 0; g < GUARD; g++)
            if (lo[g] != GUARD_BYTE || hi[g] != GUARD_BYTE) {
                fail("write outside the array");
                break;
            }
        if (arrays[i].kind == IN) {
            if (memcmp(arrays[i].data, arrays[i].pristine, arrays[i].bytes))
                fail("input was modified");
        } else if (arrays[i].kind != SCRATCH && arrays[i].is_long) {
            const long *got = arrays[i].data, *want = arrays[i].want;
            long wrong = 0;
            for (long k = 0; k < n; k++)
                wrong += got[k] != want[k];
            if (wrong) {
                fail("%ld/%ld wrong", wrong, n);
                worst = INFINITY;
            }
        } else if (arrays[i].kind != SCRATCH) {
            double e = normwise_error(arrays[i].data, arrays[i].want, n);
            if (!(e <= worst))
                worst = e;
            if (!(e <= BENCH_RTOL))
                fail("normwise error %g", e);
        }
    }
    return worst;
}

static double now(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e9 + t.tv_nsec;
}

static const struct {
    const char *name;
    unsigned type;
    unsigned long config;
} counters[NCOUNTERS] = {
    {"cycles", PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES},
    {"instructions", PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS},
    {"branch_misses", PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES},
    {"l1d_misses", PERF_TYPE_HW_CACHE,
     PERF_COUNT_HW_CACHE_L1D | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16},
    {"l2_misses", PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES},
};
static int group_fd = -1, multiplexed;

static void open_counters(void)
{
    for (int i = 0; i < NCOUNTERS; i++) {
        struct perf_event_attr pe = {.type = counters[i].type, .size = sizeof pe, .config = counters[i].config,
                                     .exclude_kernel = 1, .exclude_hv = 1, .disabled = i == 0,
                                     .read_format = PERF_FORMAT_GROUP | PERF_FORMAT_TOTAL_TIME_ENABLED |
                                                    PERF_FORMAT_TOTAL_TIME_RUNNING};
        int fd = syscall(SYS_perf_event_open, &pe, 0, -1, i ? group_fd : -1, 0);
        if (fd < 0) {
            if (group_fd >= 0)
                close(group_fd);
            group_fd = -1;
            return;
        }
        if (!i)
            group_fd = fd;
    }
    ioctl(group_fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
}

static void read_counters(double *v)
{
    struct {
        unsigned long nr, enabled, running, value[NCOUNTERS];
    } g;
    if (group_fd < 0 || read(group_fd, &g, sizeof g) != sizeof g) {
        for (int i = 0; i < NCOUNTERS; i++)
            v[i] = NAN;
        return;
    }
    if (g.running != g.enabled)
        multiplexed = 1;
    for (int i = 0; i < NCOUNTERS; i++)
        v[i] = g.value[i];
}

static int by_value(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

static long huge_kb(void)
{
    char line[256];
    long kb = -1;
    FILE *f = fopen("/proc/self/smaps_rollup", "r");
    while (f && fgets(line, sizeof line, f))
        if (sscanf(line, "AnonHugePages: %ld", &kb) == 1)
            break;
    if (f)
        fclose(f);
    return kb;
}

static const char *json_number(double v)
{
    static char buf[32];
    if (!isfinite(v))
        return "null";
    snprintf(buf, sizeof buf, "%g", v);
    return buf;
}

static double sample(int batch, double *count)
{
    double c0[NCOUNTERS], c1[NCOUNTERS];
    restore();
    read_counters(c0);
    double t0 = now();
    for (int b = 0; b < batch; b++)
        call(0);
    double t = now() - t0;
    read_counters(c1);
    for (int i = 0; i < NCOUNTERS; i++)
        count[i] = (c1[i] - c0[i]) / batch;
    return t / batch;
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3 || (strcmp(argv[1], "small") && strcmp(argv[1], "large")) ||
        (argc == 3 && strcmp(argv[2], "check"))) {
        fprintf(stderr, "usage: %s small|large [check]\n", argv[0]);
        return 2;
    }
    setup(!strcmp(argv[1], "large"));
    for (int i = 0; i < narrays; i++) {
        arrays[i].pristine = malloc(arrays[i].bytes);
        memcpy(arrays[i].pristine, arrays[i].data, arrays[i].bytes);
    }

    call(1);
    for (int i = 0; i < narrays; i++) {
        arrays[i].want = malloc(arrays[i].bytes);
        memcpy(arrays[i].want, arrays[i].data, arrays[i].bytes);
    }
    restore();
    checking = 1;
    call(0);
    checking = 0;
    double err = verify();
    if (size_failures) {
        printf("{\"status\": \"wrong_large\", \"rel_err\": %s}\n", json_number(err));
        return 0;
    }
    if (argc == 3) {
        printf("{\"status\": \"ok\", \"rel_err\": %s}\n", json_number(err));
        return 0;
    }

    open_counters();

    static double t[MAX_SAMPLES], c[NCOUNTERS][MAX_SAMPLES];
    double count[NCOUNTERS];
    double first = sample(1, count);
    int batch = 1;
    if (batchable && first < BATCH_NS)
        batch = (int)(BATCH_NS / (first > 1 ? first : 1)) + 1;
    if (first < 1e8)
        sample(batch, count);
    multiplexed = 0;

    int n = 0;
    double timed = 0;
    while (n < MAX_SAMPLES) {
        t[n] = sample(batch, count);
        for (int i = 0; i < NCOUNTERS; i++)
            c[i][n] = count[i];
        timed += t[n] * batch;
        n++;
        if (timed >= MIN_TIMED_NS && (n >= MIN_SAMPLES || (n >= MIN_LONG_SAMPLES && t[0] * batch > LONG_SAMPLE_NS)))
            break;
    }
    qsort(t, n, sizeof *t, by_value);
    for (int i = 0; i < NCOUNTERS; i++)
        qsort(c[i], n, sizeof *c[i], by_value);
    printf("{\"status\": \"ok\", \"rel_err\": %s, \"batch\": %d, \"samples\": %d, \"min_ns\": %.1f, \"med_ns\": %.1f, ",
           json_number(err), batch, n, t[0], t[n / 2]);
    printf("\"min_cyc\": %s, ", json_number(multiplexed ? NAN : c[0][0]));
    printf("\"med_cyc\": %s, \"counters\": {", json_number(multiplexed ? NAN : c[0][n / 2]));
    for (int i = 0; i < NCOUNTERS; i++)
        printf("%s\"%s\": %s", i ? ", " : "", counters[i].name, json_number(multiplexed ? NAN : c[i][n / 2]));
    printf("}, \"multiplexed\": %d, \"huge_kb\": %ld}\n", multiplexed, huge_kb());
    return 0;
}

#endif
