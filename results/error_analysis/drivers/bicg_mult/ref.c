void ref_bicg(long m, long n, const double *A, double *s, double *q, const double *p, const double *r)
{
    for (long i = 0; i < m; i++)
        s[i] = 0.0;
    for (long i = 0; i < n; i++) {
        q[i] = 0.0;
        for (long j = 0; j < m; j++) {
            s[j] = s[j] + r[i] * A[i * m + j];
            q[i] = q[i] + A[i * m + j] * p[j];
        }
    }
}
