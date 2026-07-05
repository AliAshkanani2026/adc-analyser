#include <stdio.h>
#include <stdlib.h>
#include "adc.h"
#include "io.h"

// main.c
// my main function to run the program
int main(int argc, char *argv[]) {
    printf("--- Sensor Data Analyser ---\n");
    printf("Student Project\n\n");

    // check if we got the filename
    if (argc < 2) {
        printf("error: please provide the file name\n");
        return 1;
    }

    // load data from the file
    ADCRecord *records = NULL;
    uint32_t count = read_binary_file(argv[1], &records);
    if (count == 0 || records == NULL) {
        printf("could not read file properly!\n");
        return 1;
    }
    printf("\nTotal records loaded: %u\n\n", count);

    // calculate stats for each channel
    ChannelStats stats[NUM_CHANNELS];
    compute_channel_stats(records, count, stats);

    printf("Stats:\n");
    int ch;
    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        printf("CH%d: mean: %.2f, min: %.2f, max: %.2f, "
               "dev: %.2f, faults: %d, over: %d, under: %d\n",
               ch,
               stats[ch].mean_voltage, stats[ch].min_voltage,
               stats[ch].max_voltage,  stats[ch].std_dev,
               stats[ch].fault_count,  stats[ch].overvoltage_count,
               stats[ch].undervoltage_count);
    }
    printf("\n");

    // check sequence numbers
    IntegrityReport report;
    check_integrity(records, count, &report);
    if (report.total_gaps == 0)
        printf("No gaps found in sequence.\n\n");
    else
        printf("Warning: %d gaps and %d missing records.\n\n",
               report.total_gaps, report.total_missing);

    // save results to text files
    write_results("results.txt", records, count, stats, &report);
    write_fault_report("fault_report.txt", records, count);

    // free memory
    free(records);
    records = NULL;

    printf("finished writing files.\n");
    return 0;
}
