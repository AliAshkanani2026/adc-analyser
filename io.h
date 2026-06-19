#ifndef IO_H
#define IO_H
#include <stdint.h>
#include "adc.h"

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t channel_count;
    uint32_t record_count;
    uint32_t sample_rate_hz;
    uint8_t  reserved[8];
} FileHeader;
#pragma pack(pop)

#define MAGIC_NUMBER  0xADC1BEEF
#define EXPECTED_VER  1
#define EXPECTED_CH   4
#define EXPECTED_SR   1000

uint32_t read_binary_file(const char *filename, ADCRecord **records);
void write_results(const char *filename, ADCRecord *records, uint32_t count, ChannelStats stats[NUM_CHANNELS], IntegrityReport *report);
void write_fault_report(const char *filename, ADCRecord *records, uint32_t count);
#endif
