#include <math.h>

double ref_nrm2(long n, const double *x)
{
    double s = 0.0;
    for (long i = 0; i < n; i++)
        s += x[i] * x[i];
    return sqrt(s);
}
