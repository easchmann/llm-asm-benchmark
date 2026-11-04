void ref_2mm(long ni, long nj, long nk, long nl, double alpha, double beta, double *tmp,
             const double *A, const double *B, const double *C, double *D)
{
    for (long i = 0; i < ni; i++)
        for (long j = 0; j < nj; j++) {
            tmp[i * nj + j] = 0.0;
            for (long k = 0; k < nk; k++)
                tmp[i * nj + j] += beta * A[i * nk + k] * B[k * nj + j];
        }
    for (long i = 0; i < ni; i++)
        for (long j = 0; j < nl; j++) {
            D[i * nl + j] *= beta;
            for (long k = 0; k < nj; k++)
                D[i * nl + j] += tmp[i * nj + k] * C[k * nl + j];
        }
}
