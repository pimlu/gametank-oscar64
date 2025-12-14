#pragma once

#include "geof.h"
#include "unitf.h"

// Constants for const initializers (Oscar64 doesn't understand macros/function calls)
#define GEOF_ZERO 0
#define GEOF_SIN_START 0
#define GEOF_SIN_STEP 32

#define GEOF_RECIP_START 256
#define GEOF_RECIP_STEP 16

#define GEOF_PI 16384

#define UNITF_ZERO 0
#define UNITF_ALMOST_ONE 0xffff
#define UNITF_RECIP_LAST 3855

// Camera constants
#define GEOF_CAMERA_PAN_SPEED -307
#define GEOF_CAMERA_PITCH_SPEED -307
#define GEOF_CAMERA_TRAVEL_SPEED 51
#define GEOF_CAMERA_HORIZON_Z 512

// Cube constants
#define GEOF_CUBE_SCALE 32

// Projection constants
#define GEOF_PROJ_NEAR 256
#define GEOF_PROJ_FAR 4352
#define GEOF_PROJ_DEF_PX 25052
#define GEOF_PROJ_DEF_PY 25052

// Sin/Cos constants
#define GEOF_SIN_THRESHOLD 8192
#define GEOF_SIN_RANGE 16384
#define GEOF_SIN_NEG_THRESHOLD -8192
#define GEOF_SIN_VAL1 -24576
#define GEOF_ANGLE_WRAP 32768

