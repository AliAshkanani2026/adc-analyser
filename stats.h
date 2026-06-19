#ifndef STATS_H
#define STATS_H
#include <stdint.h>
float stats_mean(const float *values, uint32_t n);
float stats_min(const float *values, uint32_t n);
float stats_max(const float *values, uint32_t n);
float stats_std_dev(const float *values, uint32_t n);
#endif
