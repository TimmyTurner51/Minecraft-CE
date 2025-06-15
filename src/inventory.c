#include "inventory.h"
#include "render.h"
#include "blocks.h"
#include <graphx.h>
#include <string.h>
#include <stdio.h>


// fix out of scope error
int inventoryVals[27] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int inventoryCounts[27] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int hotbarVals[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int hotbarSlot = 0;


void init_inventory() {
    for (int i = 0; i < 27; i++) {
        inventoryVals[i] = 0;
        inventoryCounts[i] = 0;
    }
}

void init_hotbar() {
    hotbarSlot = 0;
    // Initialize global hotbarVals directly (not local)
    // First 5 = block IDs, next 5 = block counts
    hotbarVals[0] = 0;
    hotbarVals[1] = 1;
    hotbarVals[2] = 2;
    hotbarVals[3] = 3;
    hotbarVals[4] = 4;
    hotbarVals[5] = 0;
    hotbarVals[6] = 64;
    hotbarVals[7] = 64;
    hotbarVals[8] = 64;
    hotbarVals[9] = 64;
}

void render_hotbar(void) {
    gfx_SetTextFGColor(0); // Text color
    const int slotSize = 25;
    const int numSlots = 5;
    const int hotbarWidth = slotSize * numSlots;
    const int startX = (SCREEN_W - hotbarWidth) / 2;
    const int startY = 214;

    const int w = 7; // half-width
    const int h = 5; // vertical tilt

    const int cxOffset = slotSize / 2;
    const int cyOffset = slotSize / 2 + 2;


    gfx_SetColor(148);
    gfx_FillRectangle(startX, startY, hotbarWidth, slotSize);

    for (int i = 0; i < numSlots; i++) {
        int slotX = startX + i * slotSize;
        int slotY = startY;

        int cx = slotX + cxOffset;
        int cy = slotY + cyOffset - (h / 2);

        // Slot border
        gfx_SetColor(0);
        if (i == hotbarSlot) gfx_SetColor(254);
        gfx_Rectangle(slotX, slotY, slotSize, slotSize);
        gfx_Rectangle(slotX + 1, slotY + 1, slotSize - 2, slotSize - 2);

        int blockID = hotbarVals[i];
        if (blockID < 1 || blockID >= NUM_BLOCK_TYPES) continue;

        // Per-face color pairs
        uint8_t topLight   = block_getColor(blockID, 1, SHADE_LIGHT);
        uint8_t topDark    = block_getColor(blockID, 1, SHADE_DARK);
        uint8_t leftLight  = block_getColor(blockID, 3, SHADE_LIGHT);
        uint8_t leftDark   = block_getColor(blockID, 3, SHADE_DARK);
        uint8_t rightLight = block_getColor(blockID, 4, SHADE_LIGHT);
        uint8_t rightDark  = block_getColor(blockID, 4, SHADE_DARK);

        // Top face (diamond split into 2)
        gfx_SetColor(topLight);
        gfx_FillTriangle(cx,     cy - h * 2,
                         cx,     cy,
                         cx - w, cy - h);

        gfx_SetColor(topDark);
        gfx_FillTriangle(cx,     cy - h * 2,
                         cx,     cy,
                         cx + w, cy - h);

        // Left face (trapezoid split into 2)
        gfx_SetColor(leftLight);
        gfx_FillTriangle(cx - w, cy - h,
                         cx,     cy,
                         cx - w, cy + h);

        gfx_SetColor(leftDark);
        gfx_FillTriangle(cx - w, cy + h,
                         cx,     cy,
                         cx,     cy + h * 2);

        // Right face (trapezoid split into 2)
        gfx_SetColor(rightLight);
        gfx_FillTriangle(cx + w, cy - h,
                         cx,     cy,
                         cx + w, cy + h);

        gfx_SetColor(rightDark);
        gfx_FillTriangle(cx + w, cy + h,
                         cx,     cy,
                         cx,     cy + h * 2);

        // Get block count from hotbarVals[5+i]
        int count = hotbarVals[5 + i];
        if (count > 0) {
            char text[4];
            sprintf(text, "%d", count);

            int textWidth = gfx_GetStringWidth(text);
            int textX = slotX + slotSize - textWidth - 2;       // 2px padding from right
            int textY = slotY + slotSize - 9;     // 1px padding from bottom

            gfx_PrintStringXY(text, textX, textY);
        }
    }
}
