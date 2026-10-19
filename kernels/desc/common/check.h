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

void *abi_target;
unsigned long abi_ret, abi_saved[7], abi_after[7];

static const unsigned long ABI_SENTINEL[6] = {
    0x5b5b5b5b5b5b5b01UL, 0x5b5b5b5b5b5b5b02UL, 0x5b5b5b5b5b5b5b03UL,
    0x5b5b5b5b5b5b5b04UL, 0x5b5b5b5b5b5b5b05UL, 0x5b5b5b5b5b5b5b06UL,
};
static const char *ABI_NAMES[6] = {"rbx", "rbp", "r12", "r13", "r14", "r15"};

void abi_tramp(void);
__asm__(
    ".text\n"
    ".globl abi_tramp\n"
    "abi_tramp:\n"
    "    popq abi_ret(%rip)\n"
    "    movq %rbx, abi_saved+0(%rip)\n"
    "    movq %rbp, abi_saved+8(%rip)\n"
    "    movq %r12, abi_saved+16(%rip)\n"
    "    movq %r13, abi_saved+24(%rip)\n"
    "    movq %r14, abi_saved+32(%rip)\n"
    "    movq %r15, abi_saved+40(%rip)\n"
    "    movq %rsp, abi_saved+48(%rip)\n"
    "    movabsq $0x5b5b5b5b5b5b5b01, %rbx\n"
    "    movabsq $0x5b5b5b5b5b5b5b02, %rbp\n"
    "    movabsq $0x5b5b5b5b5b5b5b03, %r12\n"
    "    movabsq $0x5b5b5b5b5b5b5b04, %r13\n"
    "    movabsq $0x5b5b5b5b5b5b5b05, %r14\n"
    "    movabsq $0x5b5b5b5b5b5b5b06, %r15\n"
    "    call *abi_target(%rip)\n"
    "    movq %rbx, abi_after+0(%rip)\n"
    "    movq %rbp, abi_after+8(%rip)\n"
    "    movq %r12, abi_after+16(%rip)\n"
    "    movq %r13, abi_after+24(%rip)\n"
    "    movq %r14, abi_after+32(%rip)\n"
    "    movq %r15, abi_after+40(%rip)\n"
    "    movq %rsp, abi_after+48(%rip)\n"
    "    movq abi_saved+48(%rip), %rsp\n"
    "    movq abi_saved+0(%rip), %rbx\n"
    "    movq abi_saved+8(%rip), %rbp\n"
    "    movq abi_saved+16(%rip), %r12\n"
    "    movq abi_saved+24(%rip), %r13\n"
    "    movq abi_saved+32(%rip), %r14\n"
    "    movq abi_saved+40(%rip), %r15\n"
    "    pushq abi_ret(%rip)\n"
    "    ret\n");

#define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))

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

static void expect_abi(void)
{
    for (int i = 0; i < 6; i++)
        if (abi_after[i] != ABI_SENTINEL[i])
            fail("callee-saved register %s not preserved", ABI_NAMES[i]);
    if (abi_after[6] != abi_saved[6])
        fail("stack pointer not restored (off by %ld bytes)", (long)(abi_after[6] - abi_saved[6]));
}

#endif
