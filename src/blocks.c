// blocks.c
#include "blocks.h"

// Fill in your actual values here.
// For each block:
//   face_colors[FACE_*][SHADE_*] = your 8-bit color/tile index
//   break_damage = how many hits till “mined”

const block_def_t block_defs[NUM_BLOCK_TYPES] = {
    [BLOCK_AIR] = {
        .face_colors = {
            [FACE_TOP]    = {0, 0},
            [FACE_BOTTOM] = {0, 0},
            [FACE_LEFT]   = {0, 0},
            [FACE_RIGHT]  = {0, 0},
            [FACE_FRONT]  = {0, 0},
            [FACE_BACK]   = {0, 0},
        },
        .break_damage = 0,
    },
    [BLOCK_GRASS] = {
        .face_colors = {
            [FACE_TOP]    = { /*SHADE_LIGHT=*/  3, /*SHADE_DARK=*/  4 },
            [FACE_BOTTOM] = { /*light=*/  98, /*dark=*/  65 },
            [FACE_LEFT]   = { /*light=*/  98, /*dark=*/  65 },
            [FACE_RIGHT]  = { /*light=*/  65, /*dark=*/  65 },
            [FACE_FRONT]  = { /*light=*/  98, /*dark=*/  98 },
            [FACE_BACK]   = { /*light=*/  98, /*dark=*/  65 },
        },
        .break_damage = 20,
    },
    [BLOCK_DIRT] = {
        .face_colors = {
            [FACE_TOP]    = {98, 65},
            [FACE_BOTTOM] = {98, 65},
            [FACE_LEFT]   = {98, 65},
            [FACE_RIGHT]  = {98, 65},
            [FACE_FRONT]  = {98, 65},
            [FACE_BACK]   = {98, 65},
        },
        .break_damage = 15,
    },
    [BLOCK_TRUNK] = {
        .face_colors = {
            [FACE_TOP]    = {65, 98},
            [FACE_BOTTOM] = {65, 98},
            [FACE_LEFT]   = {65, 98},
            [FACE_RIGHT]  = {65, 65},
            [FACE_FRONT]  = {98, 98},
            [FACE_BACK]   = {65, 98},
        },
        .break_damage = 30,
    },
    [BLOCK_LEAVES] = {
        .face_colors = {
            [FACE_TOP]    = {5, 4},
            [FACE_BOTTOM] = {4, 5},
            [FACE_LEFT]   = {5, 4},
            [FACE_RIGHT]  = {5, 4},
            [FACE_FRONT]  = {4, 5},
            [FACE_BACK]   = {4, 5},
        },
        .break_damage =  5,
    },
    // … more block_defs …
};