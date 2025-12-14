def geof_raw(value):
    return round(value * 2**8)

def unitf_raw(value):
    return round(value * 2**16)


SIN_START = 0.0
SIN_STEP = 32.0 / 256.0

RECIP_START = 1.0
RECIP_STEP = 16.0 / 256.0

MATH_PI = 64.0

# Geometry constants
CAMERA_PAN_SPEED = -1.2
CAMERA_PITCH_SPEED = -1.2
CAMERA_TRAVEL_SPEED = 0.2
CAMERA_HORIZON_Z = 2.0

CUBE_SCALE = 1.0 / 8.0

PROJ_NEAR = 1.0
PROJ_FAR = 17.0
PROJ_TAN_FOV2 = 0.5773502691896257

SIN_THRESHOLD = 32.0
SIN_RANGE = 64.0
SIN_NEG_THRESHOLD = -32.0
SIN_VAL1 = 32.0 - 128.0  # -96.0
ANGLE_WRAP = 128.0

def gen_fixed_constants():
    # Calculate projection constants
    # Screen dimensions from graphics/screen.h:
    # GRAPHICS_SCREEN_WIDTH = 128
    # GRAPHICS_SCREEN_HEIGHT = 128
    # GRAPHICS_PADDING_TOP = 7
    # GRAPHICS_PADDING_BOT = 8
    # GRAPHICS_PADDING_LEFT = 1
    # GRAPHICS_PADDING_RIGHT = 1
    # GRAPHICS_FRAME_X_HI = 128 - 1 = 127
    # GRAPHICS_FRAME_X_LO = 1
    # GRAPHICS_FRAME_W = 127 - 1 = 126
    # GRAPHICS_FRAME_Y_HI = 128 - 8 = 120
    # GRAPHICS_FRAME_Y_LO = 7
    # GRAPHICS_FRAME_H = 120 - 7 = 113
    FRAME_W = 126.0
    FRAME_H = 113.0
    ASPECT_RATIO = FRAME_W / FRAME_H
    DEF_PX = (FRAME_W / 2.0) / (ASPECT_RATIO * PROJ_TAN_FOV2)
    DEF_PY = (FRAME_H / 2.0) / PROJ_TAN_FOV2
    
    return f"""#pragma once

#include "geof.h"
#include "unitf.h"

// Constants for const initializers (Oscar64 doesn't understand macros/function calls)
#define GEOF_ZERO 0
#define GEOF_SIN_START {geof_raw(SIN_START)}
#define GEOF_SIN_STEP {geof_raw(SIN_STEP)}

#define GEOF_RECIP_START {geof_raw(RECIP_START)}
#define GEOF_RECIP_STEP {geof_raw(RECIP_STEP)}

#define GEOF_PI {geof_raw(MATH_PI)}

#define UNITF_ZERO 0
#define UNITF_ALMOST_ONE 0xffff
#define UNITF_RECIP_LAST {unitf_raw(1.0 / 17.0)}

// Camera constants
#define GEOF_CAMERA_PAN_SPEED {geof_raw(CAMERA_PAN_SPEED)}
#define GEOF_CAMERA_PITCH_SPEED {geof_raw(CAMERA_PITCH_SPEED)}
#define GEOF_CAMERA_TRAVEL_SPEED {geof_raw(CAMERA_TRAVEL_SPEED)}
#define GEOF_CAMERA_HORIZON_Z {geof_raw(CAMERA_HORIZON_Z)}

// Cube constants
#define GEOF_CUBE_SCALE {geof_raw(CUBE_SCALE)}

// Projection constants
#define GEOF_PROJ_NEAR {geof_raw(PROJ_NEAR)}
#define GEOF_PROJ_FAR {geof_raw(PROJ_FAR)}
#define GEOF_PROJ_DEF_PX {geof_raw(DEF_PX)}
#define GEOF_PROJ_DEF_PY {geof_raw(DEF_PY)}

// Sin/Cos constants
#define GEOF_SIN_THRESHOLD {geof_raw(SIN_THRESHOLD)}
#define GEOF_SIN_RANGE {geof_raw(SIN_RANGE)}
#define GEOF_SIN_NEG_THRESHOLD {geof_raw(SIN_NEG_THRESHOLD)}
#define GEOF_SIN_VAL1 {geof_raw(SIN_VAL1)}
#define GEOF_ANGLE_WRAP {geof_raw(ANGLE_WRAP)}

"""

def gen_polyfish_constants():
    # Camera initial values
    CAMERA_HEADING_THETA = 5.0
    CAMERA_PITCH_THETA = -12.0
    CAMERA_POS_X = 3.0
    CAMERA_POS_Y = 4.0
    CAMERA_POS_Z = 5.0
    
    # Cube values
    CUBE_HI = 2.0
    
    # Fish values
    FISH_BASE_Y = 1.0
    FISH_ADJUST = 1.5
    FISH_MAIN_OFFSET = 3.0
    FISH_VERTEX_X_NEG_HALF = -0.5
    FISH_VERTEX_Y_075 = 0.75
    FISH_VERTEX_Y_NEG_075 = -0.75
    FISH_VERTEX_X_1 = 1.0
    FISH_VERTEX_Y_HALF = 0.5
    FISH_VERTEX_Y_NEG_HALF = -0.5
    FISH_VERTEX_X_1_5 = 1.5
    FISH_VERTEX_Y_1 = 1.0
    FISH_VERTEX_Y_NEG_1 = -1.0
    FISH_VERTEX_Y_01 = 0.1
    
    # Horizon calculation
    HORIZON_NEG_FOUR = -4.0
    HORIZON_FIVE = 5.0
    
    return f"""#pragma once

#include "geometry/constants.h"

// Polyfish game constants
// Camera initial values
#define GEOF_CAMERA_HEADING_THETA {geof_raw(CAMERA_HEADING_THETA)}
#define GEOF_CAMERA_PITCH_THETA {geof_raw(CAMERA_PITCH_THETA)}
#define GEOF_CAMERA_POS_X {geof_raw(CAMERA_POS_X)}
#define GEOF_CAMERA_POS_Y {geof_raw(CAMERA_POS_Y)}
#define GEOF_CAMERA_POS_Z {geof_raw(CAMERA_POS_Z)}

// Cube values
#define GEOF_CUBE_HI {geof_raw(CUBE_HI)}

// Fish values
#define GEOF_FISH_BASE_Y {geof_raw(FISH_BASE_Y)}
#define GEOF_FISH_ADJUST {geof_raw(FISH_ADJUST)}
#define GEOF_FISH_MAIN_OFFSET {geof_raw(FISH_MAIN_OFFSET)}
#define GEOF_FISH_VERTEX_X_NEG_HALF {geof_raw(FISH_VERTEX_X_NEG_HALF)}
#define GEOF_FISH_VERTEX_Y_075 {geof_raw(FISH_VERTEX_Y_075)}
#define GEOF_FISH_VERTEX_Y_NEG_075 {geof_raw(FISH_VERTEX_Y_NEG_075)}
#define GEOF_FISH_VERTEX_X_1 {geof_raw(FISH_VERTEX_X_1)}
#define GEOF_FISH_VERTEX_Y_HALF {geof_raw(FISH_VERTEX_Y_HALF)}
#define GEOF_FISH_VERTEX_Y_NEG_HALF {geof_raw(FISH_VERTEX_Y_NEG_HALF)}
#define GEOF_FISH_VERTEX_X_1_5 {geof_raw(FISH_VERTEX_X_1_5)}
#define GEOF_FISH_VERTEX_Y_1 {geof_raw(FISH_VERTEX_Y_1)}
#define GEOF_FISH_VERTEX_Y_NEG_1 {geof_raw(FISH_VERTEX_Y_NEG_1)}
#define GEOF_FISH_VERTEX_Y_01 {geof_raw(FISH_VERTEX_Y_01)}

// Horizon calculation
#define GEOF_HORIZON_NEG_FOUR {geof_raw(HORIZON_NEG_FOUR)}
#define GEOF_HORIZON_FIVE {geof_raw(HORIZON_FIVE)}

"""
