
#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include <stdbool.h>

#define BLOCK_COUNT 16
#define TILE_SIZE 16

extern uint8_t *block_textures[BLOCK_COUNT];

bool load_texture_appvar(void);

#endif
