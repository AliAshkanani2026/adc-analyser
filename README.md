# ADC Sensor Log Analyser
**Student Project - UFMFGT-15-1 Programming for Engineers (Resit)**

## Project Overview
This is my C program to read and analyse the binary sensor log file (`adc_sensor_log.bin`). It goes through the data across 4 channels, spots any faults or missing sequence numbers, and writes everything into a text report.

---

## Files Included
- `main.c`: The main program loop. It handles the command-line arguments and calls the other functions.
- `adc.c` & `adc.h`: Handles the ADC stuff like converting the raw reading to voltage, detecting faults, and keeping track of the channel stats.
- `stats.c` & `stats.h`: My maths functions (mean, min, max, and standard deviation).
- `io.c` & `io.h`: Handles opening the binary file, reading the structs, and writing out the `results.txt` and `fault_report.txt` files.
- `CMakeLists.txt`: Build file so it can be compiled easily in CLion or with CMake.

---

## How to Compile and Run

### Compiling with GCC
Open your terminal in the project folder and run:
```bash
gcc -std=c99 -Wall -Wextra -o adc_analyser main.c adc.c stats.c io.c -lm
```

### Running the Program
Make sure `adc_sensor_log.bin` is in the same folder, then run:
```bash
./adc_analyser adc_sensor_log.bin
```
*(If you're on Windows, use `.\adc_analyser.exe adc_sensor_log.bin`)*

### What to expect
You should see this output in your terminal:
```text
--- Sensor Data Analyser ---
Student Project

Read 4000 records from adc_sensor_log.bin

Total records loaded: 4000

Stats:
CH0: mean: 1.65, min: 0.44, max: 2.86, dev: 0.85, faults: 0, over: 0, under: 0
CH1: mean: 1.51, min: 0.49, max: 3.20, dev: 0.72, faults: 0, over: 6, under: 0
CH2: mean: 1.79, min: 0.10, max: 2.91, dev: 0.79, faults: 0, over: 0, under: 7
CH3: mean: 1.40, min: 0.49, max: 2.31, dev: 0.64, faults: 7, over: 0, under: 0

Warning: 2 gaps and 4 missing records.

saved results to results.txt
saved faults to fault_report.txt
finished writing files.
```

---

## Repository
The source code and commit history for this project can be found on my GitHub:
[https://github.com/AliAshkanani2026/adc-analyser](https://github.com/AliAshkanani2026/adc-analyser)
