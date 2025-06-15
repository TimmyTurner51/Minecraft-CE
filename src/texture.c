
#include <fileioc.h>
#include "texture.h"

uint8_t *block_textures[BLOCK_COUNT];

bool load_texture_appvar(void) {
    ti_var_t var = ti_Open("MCTILES", "r");
    if (!var) return false;

    ti_Seek(2, SEEK_SET, var); // Skip AppVar header

    for (int i = 0; i < BLOCK_COUNT; i++) {
        block_textures[i] = ti_GetDataPtr(var) + i * (TILE_SIZE * TILE_SIZE);
    }

    ti_Close(var);
    return true;
}
