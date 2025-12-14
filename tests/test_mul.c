#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Include headers (pragmas will be ignored by clang)
#include "system/imul.h"

// Forward declaration (function not in header)
int16_t mul_shr24_sat0(int32_t x, uint16_t y, bool *oflag);

static int16_t mul_shr24_sat0_ref(int32_t x, uint16_t y, bool *oflag) {
    int64_t product = (int64_t) x * (int64_t) y;
    int64_t shifted = product >> 24;
    
    if (shifted < INT16_MIN || shifted > INT16_MAX) {
        *oflag = 1;
        return 0;
    } else {
        *oflag = 0;
        return (int16_t)shifted;
    }
}

// Test case structure
typedef struct {
    int32_t x;
    uint16_t y;
    const char *description;
} mul_test_t;

static bool test_case(const mul_test_t *test) {
    bool oflag_prod, oflag_ref;
    int16_t result_prod = mul_shr24_sat0(test->x, test->y, &oflag_prod);
    int16_t result_ref = mul_shr24_sat0_ref(test->x, test->y, &oflag_ref);
    
    if (result_prod != result_ref || oflag_prod != oflag_ref) {
        printf("FAIL: %s\n", test->description);
        printf("  x=%d (0x%08x), y=%u (0x%04x)\n", test->x, (unsigned int)test->x, test->y, test->y);
        printf("  reference: result=%d, overflow=%u\n", result_ref, oflag_ref);
        printf("  production: result=%d, overflow=%u\n", result_prod, oflag_prod);
        return false;
    }
    
    return true;
}

static bool test_case_against_ref(int32_t x, uint16_t y) {
    bool oflag_prod, oflag_ref;
    int16_t result_prod = mul_shr24_sat0(x, y, &oflag_prod);
    int16_t result_ref = mul_shr24_sat0_ref(x, y, &oflag_ref);
    
    if (result_prod != result_ref || oflag_prod != oflag_ref) {
        printf("FAIL: Reference mismatch\n");
        printf("  x=%d (0x%08x), y=%u (0x%04x)\n", x, (unsigned int)x, y, y);
        printf("  reference: result=%d, overflow=%u\n", result_ref, oflag_ref);
        printf("  production: result=%d, overflow=%u\n", result_prod, oflag_prod);
        return false;
    }
    
    return true;
}

// Fixed test cases
static const mul_test_t fixed_tests[] = {
    // Basic cases
    {0, 0, "zero * zero"},
    {1, 0, "one * zero"},
    {0, 1, "zero * one"},
    {1, 1, "one * one (shifted by 24)"},
    
    // Small positive values
    {1 << 24, 1, "1<<24 * 1"},
    {1 << 24, 2, "1<<24 * 2"},
    {2 << 24, 1, "2<<24 * 1"},
    {1 << 23, 2, "1<<23 * 2"},
    
    // Small negative values
    {(int32_t)(-1LL * (1LL << 24)), 1, "-1<<24 * 1"},
    {(int32_t)(-1LL * (1LL << 24)), 2, "-1<<24 * 2"},
    {(int32_t)(-2LL * (1LL << 24)), 1, "-2<<24 * 1"},
    {(int32_t)(-1LL * (1LL << 23)), 2, "-1<<23 * 2"},
    
    // Boundary cases for int16_t
    {INT16_MAX, 256, "INT16_MAX * 256"},
    {INT16_MIN, 256, "INT16_MIN * 256"},
    
    // Cases near int16_t boundaries
    {INT32_MAX, 1, "INT32_MAX * 1"},
    {INT32_MIN, 1, "INT32_MIN * 1"},
    
    // Large multipliers
    {1 << 24, UINT16_MAX, "1<<24 * UINT16_MAX"},
    {(int32_t)(-1LL * (1LL << 24)), UINT16_MAX, "-1<<24 * UINT16_MAX"},
    
    // Cases that might overflow
    {1 << 24, 32768, "1<<24 * 32768"},
    
    // Fractional shifts (less than 24)
    {1 << 20, 1 << 4, "1<<20 * 1<<4"},
    {1 << 16, 1 << 8, "1<<16 * 1<<8"},
    {1 << 12, 1 << 12, "1<<12 * 1<<12"},
    
    // Mixed signs
    {(int32_t)(-1LL * (1LL << 24)), 100, "-1<<24 * 100"},
    {1 << 24, 100, "1<<24 * 100"},
    
    // Edge cases around zero
    {1, (uint16_t)(1U << 24), "1 * 1<<24"},
    {-1, (uint16_t)(1U << 24), "-1 * 1<<24"},
};

// Simple RNG for fuzzing
static int32_t rng_int32(int32_t min, int32_t max) {
    int64_t range = (int64_t)max - (int64_t)min + 1;
    return min + (int32_t)(rand() % range);
}

static uint16_t rng_uint16(uint16_t min, uint16_t max) {
    int range = max - min + 1;
    return min + (uint16_t)(rand() % range);
}

int main(void) {
    printf("running fixed test suite\n");
    
    // Run fixed tests (they already compare against reference)
    for (size_t i = 0; i < sizeof(fixed_tests) / sizeof(fixed_tests[0]); i++) {
        if (!test_case(&fixed_tests[i])) {
            return 1;
        }
    }
    
    // Fuzz test comparing against reference
    const size_t ITERS = 10000000;
    srand(time(NULL));
    

    // printf("fuzzing smallest mismatch case... (%zu iters)\n", ITERS);
    // int32_t smallest_x = 0;
    // uint16_t smallest_y = 0;
    // int64_t smallest_size = INT64_MAX;
    // for (size_t i = 0; i < ITERS; i++) {
    //     int32_t x = rng_int32(-10000, 100000);
    //     uint16_t y = rng_uint16(0, UINT16_MAX);
    //     bool oflag_prod, oflag_ref;
    //     int16_t result_prod = mul_shr24_sat0(x, y, &oflag_prod);
    //     int16_t result_ref = mul_shr24_sat0_ref(x, y, &oflag_ref);
    //     int64_t size = abs(x) + (int64_t) y;
    //     if ((result_prod != result_ref || oflag_prod != oflag_ref) && size < smallest_size) {
    //         smallest_size = size;
    //         smallest_x = x;
    //         smallest_y = y;
    //     }
    // }
    // if (smallest_size != INT64_MAX) {
    //     printf("smallest mismatch case: x=%d, y=%u, size=%lld\n", smallest_x, smallest_y, smallest_size);
    // }

    printf("fuzzing against reference... (%zu iters)\n", ITERS);
    for (size_t i = 0; i < ITERS; i++) {
        int32_t x = rng_int32(INT32_MIN, INT32_MAX);
        uint16_t y = rng_uint16(0, UINT16_MAX);
        
        if (!test_case_against_ref(x, y)) {
            printf("mismatch at iteration %zu\n", i);
            return 1;
        }
    }
    
    // Test edge cases around overflow boundaries
    printf("testing overflow boundaries...\n");
    // Test with x values that when multiplied by various y might overflow
    for (int32_t x = INT16_MIN - 10; x <= INT16_MAX + 10; x++) {
        for (uint16_t y = 1; y < 100; y++) {
            if (!test_case_against_ref(x, y)) {
                return 1;
            }
        }
    }
    
    // Test with various y values at boundaries
    for (uint16_t y = 0; y < 100; y++) {
        if (!test_case_against_ref(INT16_MAX, y)) {
            return 1;
        }
        if (!test_case_against_ref(INT16_MIN, y)) {
            return 1;
        }
    }
    
    printf("passed all tests\n");
    return 0;
}

