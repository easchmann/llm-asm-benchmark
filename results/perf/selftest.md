In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/2mm/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/2mm/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/2mm/bench.c:29:9: note: in expansion of macro ‘CALL’
   29 |         CALL(mm2, ni, nj, nk, nl, 1.5, -0.75, tmp, A, B, C, D);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/3mm/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/3mm/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/3mm/bench.c:32:9: note: in expansion of macro ‘CALL’
   32 |         CALL(mm3, ni, nj, nk, nl, nm, E, A, B, F, C, D, G);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/atax/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/atax/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/atax/bench.c:24:9: note: in expansion of macro ‘CALL’
   24 |         CALL(atax, m, n, A, x, y, tmp);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/axpy/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/axpy/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/axpy/bench.c:21:9: note: in expansion of macro ‘CALL’
   21 |         CALL(axpy, n, 1.5, x, y);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/bicg/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/bicg/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/bicg/bench.c:25:9: note: in expansion of macro ‘CALL’
   25 |         CALL(bicg, m, n, A, s, q, p, r);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/dot/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/dot/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/dot/bench.c:19:42: note: in expansion of macro ‘CALL’
   19 |     result[0] = ref ? ref_dot(n, x, y) : CALL(dot, n, x, y);
      |                                          ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/gemm/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/gemm/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/gemm/bench.c:24:9: note: in expansion of macro ‘CALL’
   24 |         CALL(gemm, ni, nj, nk, 1.5, -0.75, C, A, B);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/gemver/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/gemver/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/gemver/bench.c:30:9: note: in expansion of macro ‘CALL’
   30 |         CALL(gemver, n, 1.5, -0.75, A, u1, v1, u2, v2, w, x, y, z);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/gesummv/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/gesummv/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/gesummv/bench.c:26:9: note: in expansion of macro ‘CALL’
   26 |         CALL(gesummv, n, 1.5, -0.75, A, B, tmp, x, y);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/heat3d/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/heat3d/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/heat3d/bench.c:22:9: note: in expansion of macro ‘CALL’
   22 |         CALL(heat3d, tsteps, n, A, B);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/histogram/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/histogram/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/histogram/bench.c:23:9: note: in expansion of macro ‘CALL’
   23 |         CALL(histogram, n, data, nbins, hist);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/jacobi1d/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/jacobi1d/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/jacobi1d/bench.c:22:9: note: in expansion of macro ‘CALL’
   22 |         CALL(jacobi1d, tsteps, n, A, B);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/jacobi2d/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/jacobi2d/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/jacobi2d/bench.c:22:9: note: in expansion of macro ‘CALL’
   22 |         CALL(jacobi2d, tsteps, n, A, B);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/lower_bound/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/lower_bound/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/lower_bound/bench.c:28:9: note: in expansion of macro ‘CALL’
   28 |         CALL(lower_bound, n, sorted, q, keys, idx);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/mvt/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/mvt/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/mvt/bench.c:24:9: note: in expansion of macro ‘CALL’
   24 |         CALL(mvt, n, x1, x2, y_1, y_2, A);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/nrm2/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/nrm2/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/nrm2/bench.c:18:40: note: in expansion of macro ‘CALL’
   18 |     result[0] = ref ? ref_nrm2(n, x) : CALL(nrm2, n, x);
      |                                        ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/popcount/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/popcount/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/popcount/bench.c:24:9: note: in expansion of macro ‘CALL’
   24 |         CALL(popcount, n, a, counts);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/prefix_sum/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/prefix_sum/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/prefix_sum/bench.c:23:9: note: in expansion of macro ‘CALL’
   23 |         CALL(prefix_sum, n, a, out);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/scal/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/scal/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/scal/bench.c:20:9: note: in expansion of macro ‘CALL’
   20 |         CALL(scal, n, 1.5, x);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/sort8/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/sort8/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/sort8/bench.c:23:9: note: in expansion of macro ‘CALL’
   23 |         CALL(sort8, n, a);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/spmv/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/spmv/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/spmv/bench.c:34:9: note: in expansion of macro ‘CALL’
   34 |         CALL(spmv, nrows, ncols, rowptr, col, val, x, y);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/syrk/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/syrk/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/syrk/bench.c:22:9: note: in expansion of macro ‘CALL’
   22 |         CALL(syrk, n, m, 1.5, -0.75, C, A);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/trmm/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/trmm/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/trmm/bench.c:23:9: note: in expansion of macro ‘CALL’
   23 |         CALL(trmm, m, n, 1.5, A, B);
      |         ^~~~
In file included from /home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:14,
                 from /home/elena.aschmann/llm-asm-perf/kernels/desc/trmv/bench.c:1:
/home/elena.aschmann/llm-asm-perf/kernels/desc/trmv/bench.c: In function ‘call’:
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/check.h:61:54: warning: function called through a non-compatible type
   61 | #define ABI_CALL(f, ...) (abi_target = (void *)(f), ((__typeof__(&f))(void *)abi_tramp)(__VA_ARGS__))
      |                                                     ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/common/bench.h:42:34: note: in expansion of macro ‘ABI_CALL’
   42 | #define CALL(f, ...) (checking ? ABI_CALL(f, __VA_ARGS__) : f(__VA_ARGS__))
      |                                  ^~~~~~~~
/home/elena.aschmann/llm-asm-perf/kernels/desc/trmv/bench.c:22:9: note: in expansion of macro ‘CALL’
   22 |         CALL(trmv, n, L, x, y);
      |         ^~~~
| kernel | program | small | large |
|---|---|---|---|
| 2mm | gcc-O2-v3 | ok | ok |
| 2mm | gcc-O2-scalar | ok | ok |
| 2mm | gcc-O3-native | ok | ok |
| 2mm | gcc-O3-native-fastmath | ok | ok |
| 2mm | clang-O3-native | ok | ok |
| 2mm | hand | ok | ok |
| 2mm | mutant C_from_B_stack_slot | wrong_large | wrong_large |
| 2mm | mutant skip_beta_scaling | wrong_large | wrong_large |
| 3mm | gcc-O2-v3 | ok | ok |
| 3mm | gcc-O2-scalar | ok | ok |
| 3mm | gcc-O3-native | ok | ok |
| 3mm | gcc-O3-native-fastmath | ok | ok |
| 3mm | clang-O3-native | ok | ok |
| 3mm | hand | ok | ok |
| 3mm | mutant D_from_C_stack_slot | wrong_large | wrong_large |
| 3mm | mutant E_not_zeroed | wrong_large | wrong_large |
| atax | gcc-O2-v3 | ok | ok |
| atax | gcc-O2-scalar | ok | ok |
| atax | gcc-O3-native | ok | ok |
| atax | gcc-O3-native-fastmath | ok | ok |
| atax | clang-O3-native | ok | ok |
| atax | hand | ok | ok |
| atax | mutant hardcoded_stride | wrong_large | wrong_large |
| atax | mutant modifies_input | wrong_large | wrong_large |
| atax | mutant no_zero_y | wrong_large | wrong_large |
| atax | mutant oob_write | wrong_large | wrong_large |
| axpy | gcc-O2-v3 | ok | ok |
| axpy | gcc-O2-scalar | ok | ok |
| axpy | gcc-O3-native | ok | ok |
| axpy | gcc-O3-native-fastmath | ok | ok |
| axpy | clang-O3-native | ok | ok |
| axpy | hand | ok | ok |
| axpy | mutant clobbers_rbx | wrong_large | wrong_large |
| bicg | gcc-O2-v3 | ok | ok |
| bicg | gcc-O2-scalar | ok | ok |
| bicg | gcc-O3-native | ok | ok |
| bicg | gcc-O3-native-fastmath | ok | ok |
| bicg | clang-O3-native | ok | ok |
| bicg | hand | ok | ok |
| dot | gcc-O2-v3 | ok | ok |
| dot | gcc-O2-scalar | ok | ok |
| dot | gcc-O3-native | ok | ok |
| dot | gcc-O3-native-fastmath | ok | ok |
| dot | clang-O3-native | ok | ok |
| dot | hand | ok | ok |
| dot | mutant clobbers_r12 | wrong_large | wrong_large |
| dot | mutant result_not_in_xmm0 | wrong_large | wrong_large |
| gemm | gcc-O2-v3 | ok | ok |
| gemm | gcc-O2-scalar | ok | ok |
| gemm | gcc-O3-native | ok | ok |
| gemm | gcc-O3-native-fastmath | ok | ok |
| gemm | clang-O3-native | ok | ok |
| gemm | hand | ok | ok |
| gemm | mutant A_row_stride_nj | wrong_large | wrong_large |
| gemm | mutant skip_beta_scaling | wrong_large | wrong_large |
| gemver | gcc-O2-v3 | ok | ok |
| gemver | gcc-O2-scalar | ok | ok |
| gemver | gcc-O3-native | ok | ok |
| gemver | gcc-O3-native-fastmath | ok | ok |
| gemver | clang-O3-native | ok | ok |
| gemver | hand | ok | ok |
| gemver | mutant skip_add_z | wrong_large | wrong_large |
| gemver | mutant y_from_z_stack_slot | wrong_large | wrong_large |
| gesummv | gcc-O2-v3 | ok | ok |
| gesummv | gcc-O2-scalar | ok | ok |
| gesummv | gcc-O3-native | ok | ok |
| gesummv | gcc-O3-native-fastmath | ok | ok |
| gesummv | clang-O3-native | ok | ok |
| gesummv | hand | ok | ok |
| gesummv | mutant tmp_not_zeroed | wrong_large | wrong_large |
| heat3d | gcc-O2-v3 | ok | ok |
| heat3d | gcc-O2-scalar | ok | ok |
| heat3d | gcc-O3-native | ok | ok |
| heat3d | gcc-O3-native-fastmath | ok | ok |
| heat3d | clang-O3-native | ok | ok |
| heat3d | hand | ok | ok |
| heat3d | mutant missing_centre_term | wrong_large | wrong_large |
| histogram | gcc-O2-v3 | ok | ok |
| histogram | gcc-O2-scalar | ok | ok |
| histogram | gcc-O3-native | ok | ok |
| histogram | gcc-O3-native-fastmath | ok | ok |
| histogram | clang-O3-native | ok | ok |
| histogram | hand | ok | ok |
| histogram | mutant hist_not_zeroed | wrong_large | wrong_large |
| jacobi1d | gcc-O2-v3 | ok | ok |
| jacobi1d | gcc-O2-scalar | ok | ok |
| jacobi1d | gcc-O3-native | ok | ok |
| jacobi1d | gcc-O3-native-fastmath | ok | ok |
| jacobi1d | clang-O3-native | ok | ok |
| jacobi1d | hand | ok | ok |
| jacobi1d | mutant writes_boundary | wrong_large | wrong_large |
| jacobi2d | gcc-O2-v3 | ok | ok |
| jacobi2d | gcc-O2-scalar | ok | ok |
| jacobi2d | gcc-O3-native | ok | ok |
| jacobi2d | gcc-O3-native-fastmath | ok | ok |
| jacobi2d | clang-O3-native | ok | ok |
| jacobi2d | hand | ok | ok |
| jacobi2d | mutant missing_neighbour | wrong_large | wrong_large |
| lower_bound | gcc-O2-v3 | ok | ok |
| lower_bound | gcc-O2-scalar | ok | ok |
| lower_bound | gcc-O3-native | ok | ok |
| lower_bound | gcc-O3-native-fastmath | ok | ok |
| lower_bound | clang-O3-native | ok | ok |
| lower_bound | hand | ok | ok |
| lower_bound | mutant upper_bound | wrong_large | wrong_large |
| mvt | gcc-O2-v3 | ok | ok |
| mvt | gcc-O2-scalar | ok | ok |
| mvt | gcc-O3-native | ok | ok |
| mvt | gcc-O3-native-fastmath | ok | ok |
| mvt | clang-O3-native | ok | ok |
| mvt | hand | ok | ok |
| nrm2 | gcc-O2-v3 | ok | ok |
| nrm2 | gcc-O2-scalar | ok | ok |
| nrm2 | gcc-O3-native | ok | ok |
| nrm2 | gcc-O3-native-fastmath | ok | ok |
| nrm2 | clang-O3-native | ok | ok |
| nrm2 | hand | ok | ok |
| nrm2 | mutant missing_sqrt | wrong_large | wrong_large |
| popcount | gcc-O2-v3 | ok | ok |
| popcount | gcc-O2-scalar | ok | ok |
| popcount | gcc-O3-native | ok | ok |
| popcount | gcc-O3-native-fastmath | ok | ok |
| popcount | clang-O3-native | ok | ok |
| popcount | hand | ok | ok |
| popcount | mutant low_32_bits_only | wrong_large | wrong_large |
| prefix_sum | gcc-O2-v3 | ok | ok |
| prefix_sum | gcc-O2-scalar | ok | ok |
| prefix_sum | gcc-O3-native | ok | ok |
| prefix_sum | gcc-O3-native-fastmath | ok | ok |
| prefix_sum | clang-O3-native | ok | ok |
| prefix_sum | hand | ok | ok |
| prefix_sum | mutant exclusive_scan | wrong_large | wrong_large |
| scal | gcc-O2-v3 | ok | ok |
| scal | gcc-O2-scalar | ok | ok |
| scal | gcc-O3-native | ok | ok |
| scal | gcc-O3-native-fastmath | ok | ok |
| scal | clang-O3-native | ok | ok |
| scal | hand | ok | ok |
| scal | mutant one_past_end | wrong_large | wrong_large |
| sort8 | gcc-O2-v3 | ok | ok |
| sort8 | gcc-O2-scalar | ok | ok |
| sort8 | gcc-O3-native | ok | ok |
| sort8 | gcc-O3-native-fastmath | ok | ok |
| sort8 | clang-O3-native | ok | ok |
| sort8 | hand | ok | ok |
| sort8 | mutant missing_comparator | wrong_large | wrong_large |
| spmv | gcc-O2-v3 | ok | ok |
| spmv | gcc-O2-scalar | ok | ok |
| spmv | gcc-O3-native | ok | ok |
| spmv | gcc-O3-native-fastmath | ok | ok |
| spmv | clang-O3-native | ok | ok |
| spmv | hand | ok | ok |
| spmv | mutant x_indexed_by_k | wrong_large | wrong_large |
| spmv | mutant y_wrong_stack_slot | crash | crash |
| syrk | gcc-O2-v3 | ok | ok |
| syrk | gcc-O2-scalar | ok | ok |
| syrk | gcc-O3-native | ok | ok |
| syrk | gcc-O3-native-fastmath | ok | ok |
| syrk | clang-O3-native | ok | ok |
| syrk | hand | ok | ok |
| syrk | mutant diagonal_not_scaled | wrong_large | wrong_large |
| trmm | gcc-O2-v3 | ok | ok |
| trmm | gcc-O2-scalar | ok | ok |
| trmm | gcc-O3-native | ok | ok |
| trmm | gcc-O3-native-fastmath | ok | ok |
| trmm | clang-O3-native | ok | ok |
| trmm | hand | ok | ok |
| trmm | mutant missing_alpha | wrong_large | wrong_large |
| trmv | gcc-O2-v3 | ok | ok |
| trmv | gcc-O2-scalar | ok | ok |
| trmv | gcc-O3-native | ok | ok |
| trmv | gcc-O3-native-fastmath | ok | ok |
| trmv | clang-O3-native | ok | ok |
| trmv | hand | ok | ok |
| trmv | mutant diagonal_skipped | wrong_large | wrong_large |

all good
