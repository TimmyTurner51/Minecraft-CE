#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <stdint.h>
#include <graphx.h>
#include "worldgen.h"
#include "player.h"

#define OPTIONS_COUNT 4

extern int options_values[OPTIONS_COUNT];

extern int  opt_fov_deg;            // field of view in degrees
extern int  opt_render_distance;    // how many blocks out from the player to draw
extern int  opt_lod_blocks;         // at what distance to switch to LOD impostors
extern int  opt_render_resolution;  // 0 for just lod/rectangles instead of full draw, 1+ for full resolution

void init_options(void);
void options_menu(void);

#endif // _OPTIONS_H_