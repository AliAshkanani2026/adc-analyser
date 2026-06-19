#ifndef ADC_H
#define ADC_H

#include <stdint.h>

// single record
#pragma pack(push, 1)
typedef struct {
    float    timestamp;       // time in seconds
    uint8_t  channel_id;      // channel number
    uint16_t raw_value;       // reading
    int16_t  temperature;     // temp x10
    uint8_t  status_flags;    // flags
    uint32_t sequence_number; // seq number
    uint8_t  reserved[2];     // unused padding
} ADCSample;
#pragma pack(pop)

// struct holding both raw data and calculated voltage
typedef struct {
    ADCSample raw;
    float     voltage;        // calculated voltage
} ADCRecord;

// stats for each channel
typedef struct {
    int   channel_id;
    float mean_voltage;
    float min_voltage;
    float max_voltage;
    float std_dev;
    int   fault_count;
    int   overvoltage_count;
    int   undervoltage_count;
    int   sample_count;
} ChannelStats;

// struct for checking gaps
typedef struct {
    int total_gaps;
    int total_missing;
} IntegrityReport;

#define NUM_CHANNELS  4
#define VREF          3.3f
#define ADC_MAX       4095.0f
#define OVERVOLTAGE   3.0f
#define UNDERVOLTAGE  0.3f

float adc_to_voltage(uint16_t raw_value);
void  compute_channel_stats(ADCRecord *records, uint32_t count, ChannelStats stats[NUM_CHANNELS]);
void  check_integrity(ADCRecord *records, uint32_t count, IntegrityReport *report);
void  compute_rolling_average(ADCRecord *records, uint32_t count, int channel, int window_size, float *out_averages);
void  compute_histogram(ADCRecord *records, uint32_t count, int channel, int bins[10]);
void  compute_temperature_stats(ADCRecord *records, uint32_t count, int channel, float *out_min, float *out_mean, float *out_max, int *out_over60_count);

#endif
