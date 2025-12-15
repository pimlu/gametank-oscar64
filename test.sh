#!/bin/sh
set -e

cd "$(dirname "$0")"

# Compile test with clang
# We need to include source files and define __TEST__
# Note: imul.c uses #ifdef __OSCAR64C__ to provide host-compatible implementations

# Compile the bresenham test
# -Wno-unknown-pragmas: ignore oscar64-specific pragmas
clang -D__TEST__ \
    -I. -Isrc \
    tests/bresenham_test.c \
    src/graphics/bresenham.c \
    src/graphics/bresenham_ref.c \
    src/graphics/types.c \
    src/system/i8helpers.c \
    src/system/imul.c \
    -o /tmp/bresenham_test \
    -Wall -Wextra -Werror -Wno-unknown-pragmas

# Run the test
/tmp/bresenham_test
EXIT_CODE=$?

if [ $EXIT_CODE -ne 0 ]; then
    echo "Bresenham tests failed with exit code $EXIT_CODE"
    rm -f /tmp/bresenham_test
    exit $EXIT_CODE
fi

# Cleanup bresenham test
rm -f /tmp/bresenham_test

# Compile the mul test
clang -D__TEST__ \
    -I. -Isrc \
    tests/test_mul.c \
    src/system/imul.c \
    -o /tmp/mul_test \
    -Wall -Wextra -Werror -Wno-unknown-pragmas

# Run the test
/tmp/mul_test
EXIT_CODE=$?

if [ $EXIT_CODE -ne 0 ]; then
    echo "Mul tests failed with exit code $EXIT_CODE"
    rm -f /tmp/mul_test
    exit $EXIT_CODE
fi

# Cleanup mul test
rm -f /tmp/mul_test

# Compile the LUT test
clang -D__TEST__ \
    -I. -Isrc \
    tests/test_lut.c \
    src/geometry/lut.c \
    src/geometry/sin_lut.c \
    src/geometry/recip_lut.c \
    src/geometry/sin_lut_data.c \
    src/geometry/recip_lut_data.c \
    src/geometry/geof.c \
    src/geometry/unitf.c \
    src/geometry/types.c \
    src/system/imul.c \
    -o /tmp/lut_test \
    -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-ignored-pragmas -lm

# Run the test
/tmp/lut_test
EXIT_CODE=$?

if [ $EXIT_CODE -ne 0 ]; then
    echo "LUT tests failed with exit code $EXIT_CODE"
    rm -f /tmp/lut_test
    exit $EXIT_CODE
fi

# Cleanup LUT test
rm -f /tmp/lut_test

echo "All tests passed!"
exit 0

