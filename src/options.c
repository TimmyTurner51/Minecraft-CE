#include "options.h"
#include "worldgen.h"
#include "player.h"
#include "render.h"
#include <graphx.h>
#include <keypadc.h>    // for kb_Scan(), kb_IsDown(), and kb_KeyXXX
#include <string.h>
#include <stdio.h>
#include <sys/timers.h>

// defaults:
int opt_fov_deg         =  90;
int opt_render_distance =   6;
int opt_lod_blocks      =   3;
int opt_render_resolution = 2;

// note: by setting opt_render_distance and opt_lod_blocks to the same value, LOD imposters are disabled.
// define the options text as an array of strings
const char *options_text[] = {
    "FOV: ",
    "Render Distance: ",
    "LOD Blocks: ",
    "Render Resolution: ",
};

// Option ranges
const int option_min[] = {60, 2, 1, 1};     // minimum allowed values
const int option_max[] = {120, 14, 14, 2};  // maximum allowed values

int get_slider_pixel_offset(int index, int value) {
    int range = option_max[index] - option_min[index];
    int sliderWidth = 180;
    int clampedValue = value;

    // Clamp value to prevent overshoot
    if (clampedValue < option_min[index]) clampedValue = option_min[index];
    if (clampedValue > option_max[index]) clampedValue = option_max[index];

    return ((clampedValue - option_min[index]) * sliderWidth) / range;
}

int options_values[OPTIONS_COUNT];

void init_options(void) {
    options_values[0] = opt_fov_deg;
    options_values[1] = opt_render_distance;
    options_values[2] = opt_lod_blocks;
    options_values[3] = opt_render_resolution;
}


// options menu
void options_menu(void) {
    int optionsCursor = 0;
    static int last_values[OPTIONS_COUNT] = {0};
    bool changed = true;
    int debounce = 0;

    while (!kb_IsDown(kb_KeyVars)) {
        kb_Scan();

        if (debounce == 0) {
            if (kb_IsDown(kb_KeyUp)) {
                optionsCursor = (optionsCursor - 1 + OPTIONS_COUNT) % OPTIONS_COUNT;
                debounce = 6;
            } else if (kb_IsDown(kb_KeyDown)) {
                optionsCursor = (optionsCursor + 1) % OPTIONS_COUNT;
                debounce = 6;
            }

            int *val = &options_values[optionsCursor];
            if (kb_IsDown(kb_KeyLeft) && *val > option_min[optionsCursor]) {
                (*val)--;
                debounce = 2;
            }
            if (kb_IsDown(kb_KeyRight) && *val < option_max[optionsCursor]) {
                (*val)++;
                debounce = 2;
            }

            for (int i = 0; i < OPTIONS_COUNT; i++) {
                if (options_values[i] != last_values[i]) {
                    last_values[i] = options_values[i];
                    changed = true;
                }
            }
        } else {
            debounce--;
        }

        if (changed) {
            opt_fov_deg = options_values[0];
            opt_render_distance = options_values[1];
            opt_lod_blocks = options_values[2];
            opt_render_resolution = options_values[3];

            //gfx_FillScreen(191);
            //render_wireframe(&pc, &world);
            changed = false;
        }

        // Draw options UI over wireframe
        gfx_SetTextFGColor(0);
        gfx_SetColor(107);
        gfx_FillRectangle(40, 40, 240, 160);
        gfx_SetColor(0);
        gfx_Rectangle(40, 40, 240, 160);
        gfx_Rectangle(41, 41, 238, 158);
        gfx_SetTextScale(2, 2);
        gfx_PrintStringXY("Options", 80, 50);
        gfx_SetTextScale(1, 1);

        for (int i = 0; i < OPTIONS_COUNT; i++) {
            gfx_SetColor(74);
            gfx_FillRectangle(46, 80 + (i * 20), 225, 15);
            gfx_SetColor(i == optionsCursor ? 254 : 0);
            gfx_Rectangle(46, 78 + (i * 20), 227, 17);
            gfx_Rectangle(47, 79 + (i * 20), 225, 15);
            int sliderX = 48 + get_slider_pixel_offset(i, options_values[i]);
            gfx_SetColor(148);
            gfx_FillRectangle(sliderX, 77 + (i * 20), 10, 19);
            gfx_SetColor(0);
            gfx_Rectangle(sliderX, 77 + (i * 20), 10, 19);
            char line[32];
            snprintf(line, sizeof(line), "%s%d", options_text[i], options_values[i]);
            gfx_SetTextXY(160 - gfx_GetStringWidth(line) / 2, 83 + (i * 20));
            gfx_SetTextFGColor(254);
            gfx_PrintString(line);
            if (i == 3) {
                // render resolution, 1 is basic, 2 is full
                // in the future, 1 is the same, but 2 will be low res triangles, and + would be higher res
                if (options_values[i] == 1) gfx_PrintString("Basic");
                else if (options_values[i] == 2) gfx_PrintString("Full");
            }
        }

        gfx_SwapDraw();
    }

    kb_Scan();
    opt_fov_deg = options_values[0];
    opt_render_distance = options_values[1];
    opt_lod_blocks = options_values[2];
    opt_render_resolution = options_values[3];
}
