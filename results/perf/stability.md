warm-up: 73 discarded runs in 60 s
## Stability: gcc-O2-v3 build, 20 separate processes per case, core 20

| kernel | size | median ms | CV of median | CV of min | range of medians | contended runs |
|---|---|---|---|---|---|---|
| axpy | small | 0.003249 | 0.09% | 0.32% | 0.3% | 0 |
| axpy | large | 13.03 | 0.52% | 0.55% | 2.1% | 0 |
| gemm | small | 0.4494 | 0.57% | 0.01% | 2.2% | 0 |
| dot | large | 16.59 | 0.60% | 0.60% | 2.7% | 0 |
| spmv | large | 49.52 | 0.11% | 0.21% | 0.6% | 0 |
| jacobi2d | small | 0.2659 | 0.36% | 0.04% | 1.3% | 0 |

| kernel | size | first run before warm-up vs warm median |
|---|---|---|
| axpy | small | +0.5% |
| axpy | large | +0.7% |
| gemm | small | -0.6% |
| dot | large | +2.0% |
| spmv | large | +0.1% |
| jacobi2d | small | +0.8% |

| neighbour load | kernel | size | median vs quiet | min vs quiet | flagged contended |
|---|---|---|---|---|---|
| SMT sibling, compute | axpy | small | +127.9% | +111.2% | 3/3 |
| SMT sibling, compute | axpy | large | +24.5% | +21.7% | 3/3 |
| SMT sibling, compute | gemm | small | +111.8% | +0.5% | 3/3 |
| SMT sibling, compute | dot | large | +12.0% | +12.6% | 3/3 |
| SMT sibling, compute | spmv | large | +51.0% | +55.6% | 3/3 |
| SMT sibling, compute | jacobi2d | small | +87.0% | +0.4% | 3/3 |
| same L3, memory stream | axpy | small | +0.0% | +0.0% | 3/3 |
| same L3, memory stream | axpy | large | +42.4% | +5.5% | 3/3 |
| same L3, memory stream | gemm | small | +0.9% | +0.0% | 3/3 |
| same L3, memory stream | dot | large | +12.6% | +5.3% | 3/3 |
| same L3, memory stream | spmv | large | +44.3% | +33.6% | 3/3 |
| same L3, memory stream | jacobi2d | small | +0.1% | +0.2% | 3/3 |
| same NUMA node, memory stream | axpy | small | +0.2% | +0.1% | 0/3 |
| same NUMA node, memory stream | axpy | large | +4.6% | +1.9% | 0/3 |
| same NUMA node, memory stream | gemm | small | +0.0% | +0.0% | 0/3 |
| same NUMA node, memory stream | dot | large | -0.2% | -0.2% | 0/3 |
| same NUMA node, memory stream | spmv | large | +4.3% | +2.6% | 0/3 |
| same NUMA node, memory stream | jacobi2d | small | -0.1% | -0.0% | 0/3 |
