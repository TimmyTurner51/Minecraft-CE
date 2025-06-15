#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "worldgen.h"  // now includes player_t and chunk_t

#define FP_SHIFT    8
#define ROT_STEP    5
#define MOVE_STEP   (1 << FP_SHIFT)
#define TAN_HALF_FOV  (1 << FP_SHIFT)

void world_set_block(chunk_t *chunk, int x, int y, int z, int face, uint8_t blockID);
int world_get_block(chunk_t *chunk, int x, int y, int z, int *blockID, int *face);
void player_init(player_t *p);
void player_update(player_t *p);

#endif // _PLAYER_H_