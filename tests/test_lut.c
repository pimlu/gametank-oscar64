#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>

// Include headers (pragmas will be ignored by clang)
#include "geometry/sin_lut.h"
#include "geometry/recip_lut.h"
#include "geometry/geof.h"
#include "geometry/unitf.h"
#include "geometry/constants.h"

// Test result structure
typedef struct {
    int16_t input_value;
    double max_abs_diff;
    double lut_result;
    double expected_result;
} test_result_t;

// Helper to convert geof_t to double
// GEOF_PI = 16384 represents π radians
// So 1 geof unit = π / 16384 radians
// Converting: radians = geof_val * π / 16384
static double geof_to_double_sin(int16_t geof_val) {
    // GEOF_PI = 16384 represents π
    return (double)geof_val * M_PI / 16384.0;
}

static double geof_to_double_recip(int16_t geof_val) {
    // recip calculates 1/x for x in [1.0, 17.0)
    // start=256, step=16, so geof_val=256 -> x=1.0
    // Each step of 16 = 0.0625 in the domain
    // x = 1.0 + (geof_val - 256) / 256.0
    return 1.0 + ((double)(geof_val - 256) / 256.0);
}

// Helper to convert unitf_t to double
// unitf_t is uint16_t representing [0, 1) range
// 0x0000 = 0.0, 0xffff ≈ 1.0
static double unitf_to_double(uint16_t unitf_val) {
    return (double)unitf_val / 65536.0;
}

// Helper to convert iunitf_t to double
static double iunitf_to_double(iunitf_t val) {
    double result = unitf_to_double(unitf_get_raw(val.val));
    return val.negated ? -result : result;
}

// Test function type
typedef struct {
    const char *name;
    double (*geof_to_double)(int16_t);
    double (*expected_func)(double);
    double (*lut_func)(geof_t);
    int16_t range_start;  // Start of valid range (inclusive)
    int16_t range_end;    // End of valid range (exclusive)
} test_func_t;

// Wrapper functions to call the LUT functions
static double sin_lut_wrapper(geof_t x) {
    iunitf_t result = geometry_sin(x);
    return iunitf_to_double(result);
}

static double cos_lut_wrapper(geof_t x) {
    iunitf_t result = geometry_cos(x);
    return iunitf_to_double(result);
}

static double recip_lut_wrapper(geof_t x) {
    unitf_t result = geometry_recip(x);
    return unitf_to_double(unitf_get_raw(result));
}

// Conversion functions
static double sin_expected(double x_rad) {
    return sin(x_rad);
}

static double cos_expected(double x_rad) {
    return cos(x_rad);
}

static double recip_expected(double x) {
    return 1.0 / x;
}


// Generic test function
static test_result_t test_lut_function(const test_func_t *test_func) {
    test_result_t result = {
        .input_value = 0,
        .max_abs_diff = 0.0,
        .lut_result = 0.0,
        .expected_result = 0.0
    };
    
    // Test only in-range values
    for (int32_t i = test_func->range_start; i < test_func->range_end; i++) {
        int16_t geof_val = (int16_t)i;
        geof_t x = geof_from_raw(geof_val);
        
        // Convert to double domain
        double x_double = test_func->geof_to_double(geof_val);
        
        // Get expected result
        double expected = test_func->expected_func(x_double);
        
        // Skip if expected is invalid (inf, nan, etc)
        if (!isfinite(expected)) {
            continue;
        }
        
        // Get LUT result
        double lut_result = test_func->lut_func(x);
        
        // Calculate absolute difference
        double abs_diff = fabs(lut_result - expected);
        
        // Track maximum
        if (abs_diff > result.max_abs_diff) {
            result.max_abs_diff = abs_diff;
            result.input_value = geof_val;
            result.lut_result = lut_result;
            result.expected_result = expected;
        }
    }
    
    return result;
}

int main(void) {
    printf("Testing lookup tables for in-range input values\n");
    printf("================================================\n\n");
    
    // Define test functions
    test_func_t sin_test = {
        .name = "sin",
        .geof_to_double = geof_to_double_sin,
        .expected_func = sin_expected,
        .lut_func = sin_lut_wrapper,
        .range_start = -16384,  // [-π, π] in geof units
        .range_end = 16385      // exclusive end (inclusive up to 16384)
    };
    
    test_func_t cos_test = {
        .name = "cos",
        .geof_to_double = geof_to_double_sin,  // Same scaling as sin
        .expected_func = cos_expected,
        .lut_func = cos_lut_wrapper,
        .range_start = -16384,  // [-π, π] in geof units
        .range_end = 16385      // exclusive end (inclusive up to 16384)
    };
    
    test_func_t recip_test = {
        .name = "recip (1/x)",
        .geof_to_double = geof_to_double_recip,
        .expected_func = recip_expected,
        .lut_func = recip_lut_wrapper,
        .range_start = 256,     // x in [1.0, 17.0)
        .range_end = 4352       // exclusive end
    };
    
    // Run tests
    printf("Testing sin...\n");
    test_result_t sin_result = test_lut_function(&sin_test);
    
    printf("Testing cos...\n");
    test_result_t cos_result = test_lut_function(&cos_test);
    
    printf("Testing recip (1/x)...\n");
    test_result_t recip_result = test_lut_function(&recip_test);
    
    // Print results
    printf("\n================================================\n");
    printf("RESULTS\n");
    printf("================================================\n\n");
    
    printf("sin(x):\n");
    printf("  Max absolute difference: %.10f\n", sin_result.max_abs_diff);
    printf("  Input value (geof_t): %d (0x%04x)\n", sin_result.input_value, (unsigned int)(uint16_t)sin_result.input_value);
    printf("  Input value (radians): %.10f\n", geof_to_double_sin(sin_result.input_value));
    printf("  LUT result: %.10f\n", sin_result.lut_result);
    printf("  Expected result: %.10f\n", sin_result.expected_result);
    printf("  Relative error: %.6f%%\n\n", 
           100.0 * sin_result.max_abs_diff / fmax(fabs(sin_result.expected_result), DBL_MIN));
    
    printf("cos(x):\n");
    printf("  Max absolute difference: %.10f\n", cos_result.max_abs_diff);
    printf("  Input value (geof_t): %d (0x%04x)\n", cos_result.input_value, (unsigned int)(uint16_t)cos_result.input_value);
    printf("  Input value (radians): %.10f\n", geof_to_double_sin(cos_result.input_value));
    printf("  LUT result: %.10f\n", cos_result.lut_result);
    printf("  Expected result: %.10f\n", cos_result.expected_result);
    printf("  Relative error: %.6f%%\n\n", 
           100.0 * cos_result.max_abs_diff / fmax(fabs(cos_result.expected_result), DBL_MIN));
    
    printf("recip (1/x):\n");
    printf("  Max absolute difference: %.10f\n", recip_result.max_abs_diff);
    printf("  Input value (geof_t): %d (0x%04x)\n", recip_result.input_value, (unsigned int)(uint16_t)recip_result.input_value);
    printf("  Input value (x): %.10f\n", geof_to_double_recip(recip_result.input_value));
    printf("  LUT result: %.10f\n", recip_result.lut_result);
    printf("  Expected result: %.10f\n", recip_result.expected_result);
    printf("  Relative error: %.6f%%\n\n", 
           100.0 * recip_result.max_abs_diff / fmax(fabs(recip_result.expected_result), DBL_MIN));
    
    return 0;
}

