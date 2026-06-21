#include "adc.h"
#include "stats.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

float adc_to_voltage(uint16_t raw_value) {
    return ((float)raw_value / ADC_MAX) * VREF;
}

void compute_channel_stats(ADCRecord *records, uint32_t count, ChannelStats stats[NUM_CHANNELS]) {
    int ch;
    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        stats[ch].channel_id = ch;
        stats[ch].mean_voltage = stats[ch].min_voltage = stats[ch].max_voltage = 0.0f;
        stats[ch].std_dev = 0.0f;
        stats[ch].fault_count = stats[ch].overvoltage_count = stats[ch].undervoltage_count = 0;
        stats[ch].sample_count = 0;
    }

    uint32_t i;
    for (i = 0; i < count; i++) {
        int cid = records[i].raw.channel_id;
        if (cid >= 0 && cid < NUM_CHANNELS) stats[cid].sample_count++;
    }

    float *ch_voltages[NUM_CHANNELS];
    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        ch_voltages[ch] = (float *)malloc(stats[ch].sample_count * sizeof(float));
        if (!ch_voltages[ch]) { printf("malloc failed for channel %d\n", ch); return; }
    }

    int ch_idx[NUM_CHANNELS];
    memset(ch_idx, 0, sizeof(ch_idx));

    for (i = 0; i < count; i++) {
        ADCRecord *rec = records + i;
        int cid = rec->raw.channel_id;
        if (cid < 0 || cid >= NUM_CHANNELS) continue;
        float v = rec->voltage;
        ch_voltages[cid][ch_idx[cid]++] = v;
        if (rec->raw.status_flags & 0x01) stats[cid].fault_count++;
        if (v > OVERVOLTAGE)              stats[cid].overvoltage_count++;
        if (v < UNDERVOLTAGE)             stats[cid].undervoltage_count++;
    }

    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        uint32_t n = (uint32_t)stats[ch].sample_count;
        if (n > 0) {
            stats[ch].mean_voltage = stats_mean(ch_voltages[ch], n);
            stats[ch].min_voltage  = stats_min(ch_voltages[ch], n);
            stats[ch].max_voltage  = stats_max(ch_voltages[ch], n);
            stats[ch].std_dev      = stats_std_dev(ch_voltages[ch], n);
        }
        free(ch_voltages[ch]);
    }
}

void check_integrity(ADCRecord *records, uint32_t count, IntegrityReport *report) {
    report->total_gaps = 0;
    report->total_missing = 0;
    if (count < 2) return;
    uint32_t i;
    for (i = 1; i < count; i++) {
        uint32_t prev_seq = (records + i - 1)->raw.sequence_number;
        uint32_t curr_seq = (records + i)->raw.sequence_number;
        if (curr_seq != prev_seq + 1) {
            report->total_gaps++;
            int missing = (int)(curr_seq - prev_seq) - 1;
            if (missing > 0) report->total_missing += missing;
        }
    }
}

void compute_rolling_average(ADCRecord *records, uint32_t count, int channel, int window_size, float *out_averages) {
    float *ch_v = (float *)malloc(count * sizeof(float));
    if (!ch_v) return;
    uint32_t n = 0, i;
    for (i = 0; i < count; i++)
        if ((records + i)->raw.channel_id == channel)
            ch_v[n++] = (records + i)->voltage;
    double running_sum = 0.0;
    uint32_t j;
    for (j = 0; j < n; j++) {
        running_sum += ch_v[j];
        if ((int)j >= window_size) running_sum -= ch_v[j - window_size];
        int actual_window = (j < (uint32_t)window_size) ? (int)(j + 1) : window_size;
        out_averages[j] = (float)(running_sum / actual_window);
    }
    free(ch_v);
}

void compute_histogram(ADCRecord *records, uint32_t count, int channel, int bins[10]) {
    int b;
    for (b = 0; b < 10; b++) bins[b] = 0;
    float bin_width = VREF / 10.0f;
    uint32_t i;
    for (i = 0; i < count; i++) {
        if ((records + i)->raw.channel_id != channel) continue;
        float v = (records + i)->voltage;
        int idx = (int)(v / bin_width);
        if (idx < 0) idx = 0;
        if (idx > 9) idx = 9;
        bins[idx]++;
    }
}

void compute_temperature_stats(ADCRecord *records, uint32_t count, int channel,
                               float *out_min, float *out_mean, float *out_max, int *out_over60_count) {
    *out_min = 9999.0f; *out_max = -9999.0f; *out_mean = 0.0f; *out_over60_count = 0;
    float *temps = (float *)malloc(count * sizeof(float));
    if (!temps) return;
    uint32_t n = 0, i;
    for (i = 0; i < count; i++) {
        if ((records + i)->raw.channel_id != channel) continue;
        float temp_c = (records + i)->raw.temperature / 10.0f;
        temps[n++] = temp_c;
        if (temp_c > 60.0f) (*out_over60_count)++;
    }
    if (n > 0) {
        *out_min  = stats_min(temps, n);
        *out_max  = stats_max(temps, n);
        *out_mean = stats_mean(temps, n);
    }
    free(temps);
}
