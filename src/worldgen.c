#include "worldgen.h"
#include <stdio.h>
#include <stdlib.h> // Include this header

// ground level increments from 16 to 0, or top to bottom

void generate_chunk(chunk_t *chunk) {
    int x, y, z;
    for (x = 0; x < CHUNK_SIZE; x++) {
        for (z = 0; z < CHUNK_SIZE; z++) {
            // simple “hill” function – you can swap this for noise, lookup tables, etc.
            int height = (x * z) % (CHUNK_SIZE);
            for (y = 0; y < CHUNK_SIZE; y++) {
            //chunk->blocks[x][y][z] = (y <= height) ? 1 : 0;
            // flat world
                if (y == GROUND_LEVEL) {
                    chunk->blocks[x][y][z] = 1; // ground block
                } else if (y < GROUND_LEVEL) {
                    chunk->blocks[x][y][z] = 2; // dirt block
                } else {
                    chunk->blocks[x][y][z] = 0; // air block
                }
            }
        }
    }
    // go through the chunk and generate extras like ores and trees
    for (x = 0; x < CHUNK_SIZE; x++) {
        for (z = 0; z < CHUNK_SIZE; z++) {
            
            // generate trees
            if (rand() % 40 == 0) { // 10% chance to generate a tree
                for (y = GROUND_LEVEL; y < GROUND_LEVEL + 5; y++) {
                    chunk->blocks[x][y][z] = 3; // tree block
                }
                // surround with leaves
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        if (dx == 0 && dz == 0) continue; // skip the trunk
                        if (x + dx >= 0 && x + dx < CHUNK_SIZE &&
                            z + dz >= 0 && z + dz < CHUNK_SIZE) {
                            chunk->blocks[x + dx][GROUND_LEVEL + 4][z + dz] = 4; // leaf block
                        }
                    }
                }
                // final top leaf block
                    chunk->blocks[x][GROUND_LEVEL + 5][z] = 4; // leaf block
            }


        }
    }
}