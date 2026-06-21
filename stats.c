#include "stats.h"
#include <math.h>

float stats_mean(const float *values, uint32_t n) {
    if (n == 0) return 0.0f;
    double sum = 0.0;
    const float *p = values;
    uint32_t i;
    for (i = 0; i < n; i++) { sum += *p; p++; }
    return (float)(sum / n);
}

float stats_min(const float *values, uint32_t n) {
    if (n == 0) return 0.0f;
    float minimum = *values;
    const float *p = values + 1;
    uint32_t i;
    for (i = 1; i < n; i++) { if (*p < minimum) minimum = *p; p++; }
    return minimum;
}

float stats_max(const float *values, uint32_t n) {
    if (n == 0) return 0.0f;
    float maximum = *values;
    const float *p = values + 1;
    uint32_t i;
    for (i = 1; i < n; i++) { if (*p > maximum) maximum = *p; p++; }
    return maximum;
}

float stats_std_dev(const float *values, uint32_t n) {
    if (n < 2) return 0.0f;
    float mean = stats_mean(values, n);
    double sum_sq = 0.0;
    const float *p = values;
    uint32_t i;
    for (i = 0; i < n; i++) {
        double diff = (double)(*p) - (double)mean;
        sum_sq += diff * diff;
        p++;
    }
    return (float)sqrt(sum_sq / n);
}
