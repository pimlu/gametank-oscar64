#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Include headers (pragmas will be ignored by clang)
#include "system/imul.h"

// Reference implementation using standard C multiplication
static uint64_t mul32_to_64_ref(uint32_t x, uint32_t y) {
    return (uint64_t)x * (uint64_t)y;
}

// Test a single multiplication
static bool test_mul(uint32_t x, uint32_t y) {
    uint64_t expected = mul32_to_64_ref(x, y);
    uint64_t actual = mul32_to_64(x, y);
    
    if (expected != actual) {
        printf("FAIL: %u * %u\n", x, y);
        printf("  Expected: 0x%016llx (%llu)\n", (unsigned long long)expected, (unsigned long long)expected);
        printf("  Actual:   0x%016llx (%llu)\n", (unsigned long long)actual, (unsigned long long)actual);
        printf("  Difference: %lld\n", (long long)(expected - actual));
        return false;
    }
    return true;
}

// Test structure for fixed test cases
typedef struct {
    uint32_t x;
    uint32_t y;
    const char *description;
} mul_test_t;

// Fixed test suite with edge cases
static const mul_test_t test_suite[] = {
    {0, 0, "zero * zero"},
    {0, 1, "zero * one"},
    {1, 0, "one * zero"},
    {1, 1, "one * one"},
    {0xFFFFFFFF, 0, "max * zero"},
    {0, 0xFFFFFFFF, "zero * max"},
    {0xFFFFFFFF, 1, "max * one"},
    {1, 0xFFFFFFFF, "one * max"},
    {0xFFFFFFFF, 0xFFFFFFFF, "max * max"},
    {0xFFFF, 0xFFFF, "16-bit max * 16-bit max"},
    {0x10000, 0x10000, "2^16 * 2^16"},
    {0x10000, 0xFFFF, "2^16 * (2^16-1)"},
    {0xFFFF, 0x10000, "(2^16-1) * 2^16"},
    {0x80000000, 1, "2^31 * 1"},
    {1, 0x80000000, "1 * 2^31"},
    {0x80000000, 2, "2^31 * 2"},
    {2, 0x80000000, "2 * 2^31"},
    {0x80000000, 0x80000000, "2^31 * 2^31"},
    {0x12345678, 0x9ABCDEF0, "arbitrary values 1"},
    {0xABCDEF01, 0x12345678, "arbitrary values 2"},
    {0x55555555, 0xAAAAAAAA, "pattern 1"},
    {0xAAAAAAAA, 0x55555555, "pattern 2"},
    {0x0000FFFF, 0x0000FFFF, "low 16 bits"},
    {0xFFFF0000, 0xFFFF0000, "high 16 bits"},
    {0x0000FFFF, 0xFFFF0000, "low * high"},
    {0xFFFF0000, 0x0000FFFF, "high * low"},
    {0x12340000, 0x00005678, "split values 1"},
    {0x00001234, 0x56780000, "split values 2"},
};


int main(void) {
    printf("Testing mul32_to_64\n");
    printf("===================\n\n");
    
    // Test fixed test suite
    printf("Running fixed test suite (%zu tests)...\n", sizeof(test_suite) / sizeof(test_suite[0]));
    size_t passed = 0;
    size_t failed = 0;
    
    for (size_t i = 0; i < sizeof(test_suite) / sizeof(test_suite[0]); i++) {
        if (test_mul(test_suite[i].x, test_suite[i].y)) {
            passed++;
        } else {
            failed++;
            printf("  Test case: %s\n", test_suite[i].description);
        }
    }
    
    printf("Fixed tests: %zu passed, %zu failed\n\n", passed, failed);
    
    if (failed > 0) {
        return 1;
    }
    
    // Fuzz test with random inputs
    const size_t FUZZ_ITERS = 10000000;
    printf("Fuzzing with random inputs (%zu iterations)...\n", FUZZ_ITERS);
    
    // Initialize RNG with current time
    srand((unsigned int)time(NULL));
    
    size_t fuzz_passed = 0;
    size_t fuzz_failed = 0;
    
    for (size_t i = 0; i < FUZZ_ITERS; i++) {
        uint32_t x = (uint32_t)rand() | ((uint32_t)rand() << 16);
        uint32_t y = (uint32_t)rand() | ((uint32_t)rand() << 16);
        
        if (test_mul(x, y)) {
            fuzz_passed++;
        } else {
            fuzz_failed++;
            if (fuzz_failed <= 10) { // Only print first 10 failures
                printf("  Fuzz test failure #%zu\n", fuzz_failed);
            }
        }
        
        // Progress indicator -- only if FUZZ_ITERS is large enough
        if ((i + 1) % 100000000 == 0) {
            printf("  Progress: %zu/%zu (%.1f%%)\n", i + 1, FUZZ_ITERS, 
                   (double)(i + 1) * 100.0 / FUZZ_ITERS);
        }
    }
    
    printf("Fuzz tests: %zu passed, %zu failed\n\n", fuzz_passed, fuzz_failed);
    
    // Test specific edge cases that might be problematic for Karatsuba
    printf("Testing Karatsuba-specific edge cases...\n");
    
    // Cases where (x1 + x0) or (y1 + y0) overflow 16 bits
    const mul_test_t karatsuba_tests[] = {
        {0xFFFF, 0xFFFF, "overflow case 1"},
        {0xFFFF, 0x10000, "overflow case 2"},
        {0x10000, 0xFFFF, "overflow case 3"},
        {0xFFFF, 0xFFFF0000, "overflow case 4"},
        {0xFFFF0000, 0xFFFF, "overflow case 5"},
        {0x8000, 0x8000, "overflow case 6"},
        {0x8000, 0x8001, "overflow case 7"},
        {0x8001, 0x8000, "overflow case 8"},
        {0xFFFF, 0x0001FFFF, "overflow case 9"},
        {0x0001FFFF, 0xFFFF, "overflow case 10"},
    };
    
    size_t karatsuba_passed = 0;
    size_t karatsuba_failed = 0;
    
    for (size_t i = 0; i < sizeof(karatsuba_tests) / sizeof(karatsuba_tests[0]); i++) {
        if (test_mul(karatsuba_tests[i].x, karatsuba_tests[i].y)) {
            karatsuba_passed++;
        } else {
            karatsuba_failed++;
            printf("  Karatsuba test case: %s\n", karatsuba_tests[i].description);
        }
    }
    
    printf("Karatsuba-specific tests: %zu passed, %zu failed\n\n", 
           karatsuba_passed, karatsuba_failed);
    
    if (karatsuba_failed > 0) {
        return 1;
    }
    
    // Summary
    size_t total_passed = passed + fuzz_passed + karatsuba_passed;
    size_t total_failed = failed + fuzz_failed + karatsuba_failed;
    
    printf("===================\n");
    printf("Summary: %zu passed, %zu failed\n", total_passed, total_failed);
    
    if (total_failed == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
}

