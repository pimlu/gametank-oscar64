#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// Reference implementation (from C++ test)
typedef struct {
    struct graphics_screen_pos a, b;
    int E;
    int X_left;
} bresenham_reference_pos_t;

typedef struct {
    struct graphics_screen_pos a, b;
    int E;
    int X_left;
} bresenham_reference_neg_t;

typedef struct {
    bresenham_reference_pos_t pos;
    bresenham_reference_neg_t neg;
    bool is_pos;
} bresenham_reference_t;

void bresenham_reference_pos_init(bresenham_reference_pos_t *ref, 
                                   struct graphics_screen_pos a, 
                                   struct graphics_screen_pos b);

int8_t bresenham_reference_pos_iter(bresenham_reference_pos_t *ref);

void bresenham_reference_neg_init(bresenham_reference_neg_t *ref,
                                   struct graphics_screen_pos a,
                                   struct graphics_screen_pos b);

int8_t bresenham_reference_neg_iter(bresenham_reference_neg_t *ref);

void bresenham_reference_init(bresenham_reference_t *ref,
                               struct graphics_screen_pos a,
                               struct graphics_screen_pos b);

int8_t bresenham_reference_iter(bresenham_reference_t *ref);


#pragma compile("bresenham_ref.c")

