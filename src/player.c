#include "render.h"
#include "player.h"
#include "options.h"     // opt_fov_deg, opt_render_distance
#include "inventory.h"
#include <keypadc.h>    // for kb_Scan(), kb_IsDown(), and kb_KeyXXX
#include <debug.h>

void player_init(player_t *p) {
    p->x     = (CHUNK_SIZE / 2) * FP_ONE;
    p->y     = (GROUND_LEVEL + 2) * FP_ONE;
    p->z     = - (CHUNK_SIZE * 2) * FP_ONE;
    p->yaw   = 0;
    p->pitch = 0;
}

void world_set_block(chunk_t *chunk, int x, int y, int z, int face, uint8_t blockID) {
    // Adjust target block position if placing
    if (blockID != 0) {
        x += nox[face];
        y += noy[face];
        z += noz[face];
    }

    // Bounds check
    if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE) return;

        dbg_sprintf(dbgout, "Placing block %d at (%d, %d, %d) via face %d\n", blockID, x, y, z, face);
    chunk->blocks[x][y][z] = blockID;
}

int world_get_block(chunk_t *chunk, int x, int y, int z, int *blockID, int *face) {
    // bounds check
    if (x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_SIZE ||
        z < 0 || z >= CHUNK_SIZE) return 0;
    return chunk->blocks[x][y][z];
}

void player_update(player_t *p) {
    kb_Scan();

    static uint8_t lastVars = 0;
    uint8_t curVars = kb_IsDown(kb_KeyVars);

    if (curVars && !lastVars) {
        // Wait until Vars is released before entering menu
        while (kb_IsDown(kb_KeyVars)) kb_Scan();
        options_menu(); // Now enter cleanly
    }

    lastVars = curVars;
    // --- rotation (unchanged) ---
    if (kb_IsDown(kb_KeyLeft))   { p->yaw = (p->yaw + 355) % 360; }
    if (kb_IsDown(kb_KeyRight))  { p->yaw = (p->yaw +   5) % 360; }
    if (kb_IsDown(kb_KeyUp))     { if (p->pitch <  89) p->pitch +=  5; }
    if (kb_IsDown(kb_KeyDown))   { if (p->pitch > -89) p->pitch -=  5; }

    // precompute forward vector from yaw
    int idx  = ANG_IDX(p->yaw);
    int16_t sy = sin_table[idx], cy = cos_table[idx];
    int32_t step = MOVE_STEP;    // 1 block in FP units

    // move forward/back along (sin,cos)
    if (kb_IsDown(kb_KeyMode)) {      // forward
        p->x += (sy * step) / TRIG_SCALE;
        p->z += (cy * step) / TRIG_SCALE;
    }
    if (kb_IsDown(kb_KeyApps)) {      // backward
        p->x -= (sy * step) / TRIG_SCALE;
        p->z -= (cy * step) / TRIG_SCALE;
    }

    // strafe right/left along (+cos,-sin)
    if (kb_IsDown(kb_KeyStat)) {      // strafe right
        p->x += ( cy * step) / TRIG_SCALE;
        p->z -= ( sy * step) / TRIG_SCALE;
    }
    if (kb_IsDown(kb_KeyAlpha)) {     // strafe left
        p->x -= ( cy * step) / TRIG_SCALE;
        p->z += ( sy * step) / TRIG_SCALE;
    }

    // vertical flight (unchanged)
    if (kb_IsDown(kb_Key2nd))  p->y += step;
    if (kb_IsDown(kb_KeyMath)) p->y -= step;

    // hotbar slot selection
    if (kb_IsDown(kb_KeyYequ)) hotbarSlot = 0;
    if (kb_IsDown(kb_KeyWindow)) hotbarSlot = 1;
    if (kb_IsDown(kb_KeyZoom)) hotbarSlot = 2;
    if (kb_IsDown(kb_KeyTrace)) hotbarSlot = 3;
    if (kb_IsDown(kb_KeyGraph)) hotbarSlot = 4;


    // place block here
    if (kb_IsDown(kb_KeyGraphVar) && hasBestFace && hotbarVals[hotbarSlot] != 0 && hotbarVals[hotbarSlot + 5] >  0 && hotbarVals[hotbarSlot] < 64)
    {
        world_set_block(&world, selectedBlockX, selectedBlockY, selectedBlockZ, selectedFace, hotbarVals[hotbarSlot]);
        hotbarVals[hotbarSlot + 5]--;
        if (hotbarVals[hotbarSlot + 5] == 0) hotbarVals[hotbarSlot] = 0;
    }
    // delete block here
    if (kb_IsDown(kb_KeyDel) && hasBestFace)
    {
        // test if hotbarVals[hotbarSlot] is 0 or the same block as the block at selectedBlockX, selectedBlockY, selectedBlockZ
        if (hotbarVals[hotbarSlot] == 0 || world_get_block(&world, selectedBlockX, selectedBlockY, selectedBlockZ, &selectedFace, &selectedFace) == hotbarVals[hotbarSlot])
        {
            hotbarVals[hotbarSlot] = world_get_block(&world, selectedBlockX, selectedBlockY, selectedBlockZ, &selectedFace, &selectedFace);
            hotbarVals[hotbarSlot + 5]++;
            world_set_block(&world, selectedBlockX, selectedBlockY, selectedBlockZ, selectedFace, 0);
        }
    }

}