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

echo "All tests passed!"
exit 0

