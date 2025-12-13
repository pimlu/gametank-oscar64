#!/bin/sh
set -e

cd "$(dirname "$0")"

# Compile test with clang
# We need to include source files and define __TEST__

# Create a simple stub for imul functions that doesn't require 6502-specific code
cat > /tmp/imul_stub.c << 'EOF'
#include <stdint.h>
#include <stdbool.h>

// Stub implementations for host testing
uint16_t mul8_to_16(uint8_t a, uint8_t b) {
    return (uint16_t)a * (uint16_t)b;
}

int16_t imul8_to_16(int8_t x, int8_t y) {
    return (int16_t)x * (int16_t)y;
}

// Dummy init function (not needed for tests)
void mul_init(void) {
}
EOF

# Compile the test
# -Wno-unknown-pragmas: ignore oscar64-specific pragmas
# -Wno-error=unknown-pragmas: don't treat unknown pragmas as errors
clang -D__TEST__ \
    -I. -Isrc \
    tests/bresenham_test.c \
    src/graphics/bresenham.c \
    src/graphics/types.c \
    src/system/i8helpers.c \
    /tmp/imul_stub.c \
    -o /tmp/bresenham_test \
    -Wall -Wextra -Werror -Wno-unknown-pragmas

# Run the test
/tmp/bresenham_test
EXIT_CODE=$?

# Cleanup
rm -f /tmp/bresenham_test /tmp/imul_stub.c

if [ $EXIT_CODE -eq 0 ]; then
    echo "All tests passed!"
else
    echo "Tests failed with exit code $EXIT_CODE"
    exit $EXIT_CODE
fi

