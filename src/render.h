#ifndef _RENDER_H_
#define _RENDER_H_

#include <stdint.h>
#include <stdbool.h>
#include "worldgen.h"
#include "player.h"

#define SCREEN_W   320
#define SCREEN_H   240

// fixed-point scale: 1.0 == (1<<FP_SHIFT)
#define FP_SHIFT    8
#define FP_ONE      (1 << FP_SHIFT)

// Direction vectors for faces (used for face normals and neighbor offsets)
extern const int8_t nox[6];
extern const int8_t noy[6];
extern const int8_t noz[6];

// trig table scale & indexing
#define TRIG_SCALE 1024
#define ANG_SCALE   64
#define ANG_IDX(a)  ( (((a) * ANG_SCALE) / 360) & (ANG_SCALE - 1) )

// 64-entry tables live in render.c
extern const int16_t sin_table[64];
extern const int16_t cos_table[64];

// these are updated by render_wireframe() and used by player.c
extern int selectedBlockX, selectedBlockY, selectedBlockZ;
extern int selectedFace;
extern bool hasBestFace;

// draw every visible face of every block (within RENDER_DISTANCE)
void render_wireframe(const player_t *p, chunk_t *chunk);

#endif // _RENDER_H_