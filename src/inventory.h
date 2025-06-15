#ifndef INVENTORY_H
#define INVENTORY_H

#include <stdint.h>

// make inventory and hotbar global variables
extern int inventoryVals[27];
extern int inventoryCounts[27];
extern int hotbarVals[10];
extern int hotbarSlot;

void init_inventory(void);
void init_hotbar(void);
void render_hotbar(void);


#endif