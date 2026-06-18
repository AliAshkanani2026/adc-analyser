cmake_minimum_required(VERSION 3.10)
project(adc_analyser C)

# Use the C99 standard.
# C99 introduced: stdint.h (fixed-width integer types like uint32_t),
# variable-length array declarations, // comments, and more.
# We need stdint.h for uint8_t, uint16_t, uint32_t, int16_t.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable all common warnings. These catch bugs at compile time:
#   -Wall   : most important warnings (unused variables, missing returns, etc.)
#   -Wextra : additional warnings (unused parameters, etc.)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Define the executable target.
# List all .c source files that make up the program.
add_executable(adc_analyser
    main.c
    adc.c
    stats.c
    io.c
)

# Link the math library (-lm).
# sqrt() lives in libm on Linux/Unix. Without this, the linker
# will report "undefined reference to sqrt".
# On Windows with MSVC, math functions are in the C runtime and
# no separate link step is needed, but -lm is harmless there.
target_link_libraries(adc_analyser m)
