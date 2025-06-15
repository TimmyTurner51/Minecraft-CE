#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdint.h>

// Face indices
#define FACE_BOTTOM  0
#define FACE_TOP     1
#define FACE_LEFT    2
#define FACE_RIGHT   3
#define FACE_FRONT   4
#define FACE_BACK    5

// Shade indices (e.g. two variants per face)
#define SHADE_LIGHT  0
#define SHADE_DARK   1

// Enumerate your block types here:
typedef enum {
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_TRUNK,
    BLOCK_LEAVES,
    // … add more …
    NUM_BLOCK_TYPES
} block_type_t;

// All the data for one block type:
typedef struct {
    // [face][shade]
    uint8_t face_colors[6][2];
    // how many “hits” to break
    uint8_t break_damage;
} block_def_t;

// Declare your definitions array & accessors:
extern const block_def_t block_defs[NUM_BLOCK_TYPES];

/**
 * Get the color/texture index for a given block type, face, and shade.
 *
 * @param type   which block (0..NUM_BLOCK_TYPES-1)
 * @param face   FACE_TOP…FACE_BACK
 * @param shade  SHADE_LIGHT or SHADE_DARK
 * @return       the uint8_t color (or tile index) you assigned
 *               or 0 on any out-of-range argument
 */
static inline uint8_t block_getColor(block_type_t type, int face, int shade) {
    if ((unsigned)type >= NUM_BLOCK_TYPES) return 0;
    if (face < 0 || face >= 6)             return 0;
    if (shade < 0 || shade >= 2)           return 0;
    return block_defs[type].face_colors[face][shade];
}

/**
 * Get how much “damage” (hits) it takes to break this block.
 */
static inline uint8_t block_getBreakDamage(block_type_t type) {
    if ((unsigned)type >= NUM_BLOCK_TYPES) return 0;
    return block_defs[type].break_damage;
}

#endif // BLOCKS_H