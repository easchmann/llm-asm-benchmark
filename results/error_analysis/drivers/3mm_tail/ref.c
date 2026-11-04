void ref_3mm(long ni, long nj, long nk, long nl, long nm, double *E, const double *A, const double *B,
             double *F, const double *C, const double *D, double *G)
{
    for (long i = 0; i < ni; i++)
        for (long j = 0; j < nj; j++) {
            E[i * nj + j] = 0.0;
            for (long k = 0; k < nk; k++)
                E[i * nj + j] += A[i * nk + k] * B[k * nj + j];
        }
    for (long i = 0; i < nj; i++)
        for (long j = 0; j < nl; j++) {
            F[i * nl + j] = 0.0;
            for (long k = 0; k < nm; k++)
                F[i * nl + j] += C[i * nm + k] * D[k * nl + j];
        }
    for (long i = 0; i < ni; i++)
        for (long j = 0; j < nl; j++) {
            G[i * nl + j] = 0.0;
            for (long k = 0; k < nj; k++)
                G[i * nl + j] += E[i * nj + k] * F[k * nl + j];
        }
}
