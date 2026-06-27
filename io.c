#include "io.h"
#include "adc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

uint32_t read_binary_file(const char *filename, ADCRecord **records) {
    if (sizeof(FileHeader) != 24) { printf("header size is wrong\n"); return 0; }
    if (sizeof(ADCSample)  != 16) { printf("sample size is wrong\n");  return 0; }

    FILE *fp = fopen(filename, "rb");
    if (!fp) { printf("error: could not open file %s\n", filename); return 0; }

    FileHeader header;
    if (fread(&header, sizeof(FileHeader), 1, fp) != 1) { printf("error reading header\n"); fclose(fp); return 0; }
    if (header.magic != MAGIC_NUMBER) { printf("bad magic number!\n"); fclose(fp); return 0; }
    if (header.version != EXPECTED_VER) printf("warning: weird version number %u\n", header.version);

    uint32_t rec_count = header.record_count;
    *records = (ADCRecord *)malloc(rec_count * sizeof(ADCRecord));
    if (!*records) { printf("malloc failed to allocate memory\n"); fclose(fp); return 0; }

    uint32_t i, records_ok = 0;
    for (i = 0; i < rec_count; i++) {
        ADCRecord *rec = (*records) + i;
        if (fread(&rec->raw, sizeof(ADCSample), 1, fp) != 1) { printf("warning: stopped reading early at %u\n", i); break; }
        rec->voltage = adc_to_voltage(rec->raw.raw_value);
        records_ok++;
    }
    fclose(fp);
    printf("Read %u records from %s\n", records_ok, filename);
    return records_ok;
}

void write_results(const char *filename, ADCRecord *records, uint32_t count,
                   ChannelStats stats[NUM_CHANNELS], IntegrityReport *report) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { printf("could not create %s\n", filename); return; }

    fprintf(fp, "--- Results ---\n\n");
    fprintf(fp, "Dataset Info:\n");
    fprintf(fp, "Total records: %u\n", count);
    fprintf(fp, "Channels: %d\n\n", NUM_CHANNELS);

    fprintf(fp, "Channel Stats:\n");
    int ch;
    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        ChannelStats *s = stats + ch;
        fprintf(fp, "\n  Channel %d  (%d samples)\n", ch, s->sample_count);
        fprintf(fp, "    Mean voltage  : %7.4f V\n", s->mean_voltage);
        fprintf(fp, "    Min voltage   : %7.4f V\n", s->min_voltage);
        fprintf(fp, "    Max voltage   : %7.4f V\n", s->max_voltage);
        fprintf(fp, "    Std deviation : %7.4f V\n", s->std_dev);
    }

    fprintf(fp, "\nFaults Summary:\n");
    fprintf(fp, "CH  Faults  OverV  UnderV\n");
    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        ChannelStats *s = stats + ch;
        fprintf(fp, "%d   %d       %d      %d\n", ch, s->fault_count, s->overvoltage_count, s->undervoltage_count);
    }

    fprintf(fp, "\nSequence check:\n");
    if (report->total_gaps == 0) {
        fprintf(fp, "All good.\n");
    } else {
        fprintf(fp, "Gaps: %d, missing: %d\n", report->total_gaps, report->total_missing);
        uint32_t i;
        for (i = 1; i < count; i++) {
            uint32_t prev = (records + i - 1)->raw.sequence_number;
            uint32_t curr = (records + i)->raw.sequence_number;
            if (curr != prev + 1) {
                fprintf(fp, "    t=%.4f s: seq %u -> %u  (%d lost)\n",
                        (records + i)->raw.timestamp, prev, curr, (int)(curr - prev) - 1);
            }
        }
    }

    fprintf(fp, "\n\nTEMPERATURE ANALYSIS  [Extension]\n");
    fprintf(fp, "---------------------------------------\n");
    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        float tmin, tmean, tmax; int over60;
        compute_temperature_stats(records, count, ch, &tmin, &tmean, &tmax, &over60);
        fprintf(fp, "  Channel %d: min=%.1f C  mean=%.1f C  max=%.1f C  over_60C=%d\n",
                ch, tmin, tmean, tmax, over60);
    }

    fprintf(fp, "\n\nVOLTAGE HISTOGRAMS  [Extension - 10 bins, 0-3.3 V]\n");
    fprintf(fp, "---------------------------------------\n");
    float bw = VREF / 10.0f;
    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        int bins[10]; compute_histogram(records, count, ch, bins);
        fprintf(fp, "\n  Channel %d:\n", ch);
        int b;
        for (b = 0; b < 10; b++)
            fprintf(fp, "    [%.2f-%.2f V]: %d\n", b * bw, (b+1) * bw, bins[b]);
    }

    fprintf(fp, "\nEnd of results.\n");
    fclose(fp);
    printf("saved results to %s\n", filename);
}

void write_fault_report(const char *filename, ADCRecord *records, uint32_t count) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { printf("error writing %s\n", filename); return; }

    fprintf(fp, "--- Faults found ---\n");

    uint32_t i; int total = 0;
    for (i = 0; i < count; i++) {
        ADCRecord *rec = records + i;
        uint8_t flags = rec->raw.status_flags;
        float v = rec->voltage;
        if (!((flags & 0x01) || (v > OVERVOLTAGE) || (v < UNDERVOLTAGE))) continue;
        char fault_str[80] = "";
        if (flags & 0x01)    strcat(fault_str, "SENSOR_FAULT ");
        if (flags & 0x02)    strcat(fault_str, "OOR_FLAG ");
        if (v > OVERVOLTAGE) strcat(fault_str, "OVERVOLTAGE ");
        if (v < UNDERVOLTAGE)strcat(fault_str, "UNDERVOLTAGE ");
        fprintf(fp, "SEQ %u, CH %d, Time %.4f, V %.4f, Flags 0x%02X: %s\n",
                rec->raw.sequence_number, rec->raw.channel_id, rec->raw.timestamp, v, flags, fault_str);
        total++;
    }
    fprintf(fp, "\nTotal faulted records: %d\n", total);
    fclose(fp);
    printf("saved faults to %s\n", filename);
}
