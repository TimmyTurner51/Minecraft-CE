#ifndef _WORLDGEN_H_
#define _WORLDGEN_H_

#include <stdint.h>

#define CHUNK_SIZE    16
#define GROUND_LEVEL   8

// Move this struct here:
typedef struct {
    int32_t x, y, z;   // fixed-point world pos
    int16_t yaw;       // 0…359°
    int16_t pitch;     // –89…+89°
} player_t;

typedef struct {
    uint8_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
} chunk_t;

void generate_chunk(chunk_t *chunk);

// Globals
extern chunk_t world;
extern player_t pc;

#endif // _WORLDGEN_H_