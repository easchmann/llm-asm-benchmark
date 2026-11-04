void ref_gemm(long ni, long nj, long nk, double alpha, double beta, double *C, const double *A, const double *B)
{
    for (long i = 0; i < ni; i++) {
        for (long k = 0; k < nk; k++)
            for (long j = 0; j < nj; j++)
                C[i * nj + j] += alpha * A[i * nk + k] * B[k * nj + j];
        for (long j = 0; j < nj; j++)
            C[i * nj + j] *= beta;
    }
}
