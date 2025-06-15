#include <tice.h>
#include <graphx.h>
#include <keypadc.h>     // kb_Scan(), kb_IsDown(), KEY_CLEAR

#include "options.h"     // opt_fov_deg, opt_render_distance
#include "worldgen.h"    // chunk_t, generate_chunk()
#include "player.h"      // player_t, player_init(), player_update()
#include "render.h"      // render_wireframe()
#include "inventory.h"   // render_hotbar()

chunk_t world;
player_t pc;

int main(void) {
    gfx_Begin();
    gfx_SetDrawBuffer();


    init_options();
    init_inventory();
    init_hotbar();
    generate_chunk(&world);
    player_init(&pc);
    while (1) {
        kb_Scan();
        if (kb_IsDown(kb_KeyClear)) break;

        player_update(&pc);

        // sky
        gfx_SetColor(191);
        gfx_FillScreen(191);

        // wireframe blocks
        gfx_SetColor(0);
        render_wireframe(&pc, &world);

        // render hotbar
        render_hotbar();

        gfx_SwapDraw();
    }

    gfx_End();
    return 0;
}