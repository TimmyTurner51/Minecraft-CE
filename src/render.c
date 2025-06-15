#include "render.h"
#include "blocks.h"
#include "options.h"
#include <graphx.h>

// sine & cosine lookup ×1024
const int16_t sin_table[64] = {
     0,   100,   200,   297,   392,   483,   569,   650,
   724,   792,   851,   903,   946,   980,  1004,  1019,
  1024,  1019,  1004,   980,   946,   903,   851,   792,
   724,   650,   569,   483,   392,   297,   200,   100,
     0,  -100,  -200,  -297,  -392,  -483,  -569,  -650,
  -724,  -792,  -851,  -903,  -946,  -980, -1004, -1019,
 -1024, -1019, -1004,  -980,  -946,  -903,  -851,  -792,
  -724,  -650,  -569,  -483,  -392,  -297,  -200,  -100
};
const int16_t cos_table[64] = {
  1024,  1019,  1004,   980,   946,   903,   851,   792,
   724,   650,   569,   483,   392,   297,   200,   100,
     0,  -100,  -200,  -297,  -392,  -483,  -569,  -650,
  -724,  -792,  -851,  -903,  -946,  -980, -1004, -1019,
 -1024, -1019, -1004,  -980,  -946,  -903,  -851,  -792,
  -724,  -650,  -569,  -483,  -392,  -297,  -200,  -100,
     0,   100,   200,   297,   392,   483,   569,   650,
   724,   792,   851,   903,   946,   980,  1004,  1019
};
// FOV degrees from 60 to 120, step 2
const int32_t fov_proj_lookup[] = {
 46937, 42436, 38431, 34863, 31735, 28983, 26543, 24391,
 22430, 20638, 19006, 17528, 16191, 14996, 13939, 13007,
 12186, 11476, 10867, 10356,  9930,  9580,  9325,  9150,
  9051,  9025,  9078,  9203,  9402,  9675, 10023
};

// cube‐vertex offsets
static const uint8_t dx[8] = {0,1,1,0, 0,1,1,0};
static const uint8_t dy[8] = {0,0,1,1, 0,0,1,1};
static const uint8_t dz[8] = {0,0,0,0, 1,1,1,1};

// for each face: which 4 verts (in dx/dy/dz) form that quad
static const uint8_t face_verts[6][4] = {
  {0,1,5,4},  // bottom (y-)
  {3,2,6,7},  // top    (y+)
  {0,1,2,3},  // north  (z-)
  {4,5,6,7},  // south  (z+)
  {0,4,7,3},  // west   (x-)
  {1,5,6,2}   // east   (x+)
};

// neighbor offsets == face normals in integer blocks
const int8_t nox[6] = { 0,  0,  0,  0, -1,  1 };
const int8_t noy[6] = {-1,  1,  0,  0,  0,  0 };
const int8_t noz[6] = { 0,  0, -1,  1,  0,  0 };

// Project one camera-relative point to screen.
// Returns 1 if in front (zp > 0), and writes to out_x/out_y.
// Uses caller-provided proj_dist based on opt_fov_deg.
#define ASPECT_NUM 3
#define ASPECT_DEN 4

// These are updated by render_wireframe() and used by player.c
int selectedBlockX, selectedBlockY, selectedBlockZ;
int selectedFace;
bool hasBestFace = false;

// Project one camera-relative point to screen.
// Returns 1 if in front (zp>0), and writes out_x/out_y.
static int project_point(int32_t x, int32_t y, int32_t z,
                         const player_t *p,
                         int32_t proj_dist,     // ← NEW
                         int *out_x, int *out_y)
{
    // yaw
    int iy = ANG_IDX(p->yaw);
    int16_t sy = sin_table[iy], cy = cos_table[iy];
    int32_t xr = (cy * x - sy * z) / TRIG_SCALE;
    int32_t zr = (sy * x + cy * z) / TRIG_SCALE;

    // pitch
    int pitch360 = (p->pitch < 0) ? (p->pitch + 360) : p->pitch;
    int ip = ANG_IDX(pitch360);
    int16_t sp = sin_table[ip], cp = cos_table[ip];
    int32_t yr = (cp * y - sp * zr) / TRIG_SCALE;
    int32_t zp = (sp * y + cp * zr) / TRIG_SCALE;

    // Must be in front of camera
    if (zp <= (FP_ONE >> 2)) return 0;

    // Perspective projection (FIXED / FIXED = FIXED)
    int32_t sx_fp = (xr * proj_dist) / zp;
    int32_t sy_fp = (yr * proj_dist) / zp;

    // Convert fixed-point to screen coords
    *out_x = (sx_fp >> FP_SHIFT) + (SCREEN_W >> 1);
    *out_y = (SCREEN_H >> 1) - (sy_fp >> FP_SHIFT);

    return 1;
}


void render_wireframe(const player_t *p, chunk_t *chunk) {
    gfx_SetColor(0);
    // set up vars for a cursor to draw on the block in the middle of the screen
    int bestScore = 1000000;
    int bestFaceX[4], bestFaceY[4];
    const int screenCX = SCREEN_W >> 1;
    const int screenCY = SCREEN_H >> 1;
    // calculate the projection distance based on the FOV and clamp it to 60-120
    int fov_clamped = opt_fov_deg;
    if (fov_clamped < 60) fov_clamped = 60;
    if (fov_clamped > 120) fov_clamped = 120;

    int32_t proj_dist = fov_proj_lookup[(opt_fov_deg - 60) / 2];
    // cache player pos & precompute trig once
    const int32_t px   = p->x, pz = p->z;
    int iyaw            = ANG_IDX(p->yaw);
    int16_t sy_y        = sin_table[iyaw], cy_y = cos_table[iyaw];
    int pitch360        = p->pitch < 0 ? p->pitch + 360 : p->pitch;
    int ip              = ANG_IDX(pitch360);
    int16_t sp_x        = sin_table[ip], cp_x = cos_table[ip];
        // compute our render‐distance in fixed‐point from the menu variable
    int32_t renderDistFP   = opt_render_distance << FP_SHIFT;
    int32_t renderDistFPsq = renderDistFP * renderDistFP;

    // back-to-front ordering for X columns
    int x0, x1, xstep;
    if (sy_y >= 0) { x0 = CHUNK_SIZE - 1; x1 = -1;         xstep = -1; }
    else            { x0 = 0;             x1 = CHUNK_SIZE; xstep =  1; }

    // back-to-front ordering for Z columns
    int z0, z1, zstep;
    if (cy_y >= 0) { z0 = CHUNK_SIZE - 1; z1 = -1;         zstep = -1; }
    else            { z0 = 0;             z1 = CHUNK_SIZE; zstep =  1; }


    // LOD impostor parameters
    const int32_t IMP_LOD_BLOCKS = opt_lod_blocks;
    const int32_t IMP_LOD_FP     = (IMP_LOD_BLOCKS << FP_SHIFT);
    const int32_t IMP_LOD_FP_SQ  = IMP_LOD_FP * IMP_LOD_FP;
    static const uint8_t impColor[6] = { 3, 65, 139, 5, 0, 0 };

    // X → Z → Y loops
    for (int bx = x0; bx != x1; bx += xstep) {
        int32_t bx_c  = (bx << FP_SHIFT) + (FP_ONE >> 1);
        int32_t dx_fp = bx_c - px;

        for (int bz = z0; bz != z1; bz += zstep) {
            int32_t bz_c  = (bz << FP_SHIFT) + (FP_ONE >> 1);
            int32_t dz_fp = bz_c - pz;

            // circular distance‐cull
            if ((dx_fp*dx_fp + dz_fp*dz_fp) > renderDistFPsq)
                continue;

            // —— LOD impostor: far columns become circles ——
            if ((dx_fp*dx_fp + dz_fp*dz_fp) > IMP_LOD_FP_SQ) {
                // find the topmost block in this column
                int topY = CHUNK_SIZE - 1;
                while (topY >= 0 && !chunk->blocks[bx][topY][bz]) topY--;
                if (topY >= 0) {
                    // world‐space center of that block
                    int32_t wx = bx_c  - px;
                    int32_t wy = ((topY << FP_SHIFT) + (FP_ONE>>1)) - p->y;
                    int32_t wz = bz_c  - pz;

                    // project center
                    int sx, sy;
                    if (project_point(wx, wy, wz, p, proj_dist, &sx, &sy)) {
                        // compute depth for sizing
                        int32_t xr = ( cy_y * wx - sy_y * wz) / TRIG_SCALE;
                        int32_t zr = ( sy_y * wx + cy_y * wz) / TRIG_SCALE;
                        int32_t yr = ( cp_x * wy - sp_x * zr) / TRIG_SCALE;
                        int32_t zp = ( sp_x * wy + cp_x * zr) / TRIG_SCALE;
                        if (zp > 0) {
                            // radius ∝ 1/zp
                            int radius = ((proj_dist * (FP_ONE>>1)) / zp) >> FP_SHIFT;
                            // pick color
                            int bt = chunk->blocks[bx][topY][bz] - 1;
                            gfx_SetColor((bt>=0 && bt<6) ? impColor[bt] : 0);
                            gfx_FillRectangle(sx-radius, sy-radius,
                                         radius*2, radius*2);
                        }
                    }
                }
                continue;  // skip per‐face rendering
            }
            // —— end LOD impostor ——

            // scan this column top-down, draw only the first non-air block
            for (int by = CHUNK_SIZE - 1; by >= 0; by--) {
                if (!chunk->blocks[bx][by][bz]) continue;

                // test its 6 faces
                for (int f = 0; f < 6; f++) {
                    // 1) neighbor‐cull
                    int nx = bx + nox[f], ny = by + noy[f], nz = bz + noz[f];
                    if (nx>=0 && nx<CHUNK_SIZE &&
                        ny>=0 && ny<CHUNK_SIZE &&
                        nz>=0 && nz<CHUNK_SIZE &&
                        chunk->blocks[nx][ny][nz])
                    {
                        continue;
                    }

                    // 2) world‐space dot‐cull
                    //
                    //  face_center = block_origin + (nox+1)/2 * block_size
                    //             = bx*FP_ONE + ((nox[f]+1) * (FP_ONE>>1))
                    //
                    int32_t face_cx = (bx << FP_SHIFT)
                                    + ((nox[f] + 1) * (FP_ONE >> 1));
                    int32_t face_cy = (by << FP_SHIFT)
                                    + ((noy[f] + 1) * (FP_ONE >> 1));
                    int32_t face_cz = (bz << FP_SHIFT)
                                    + ((noz[f] + 1) * (FP_ONE >> 1));
                    int32_t vx  = px - face_cx,
                            vy  = p->y - face_cy,
                            vz  = p->z - face_cz;
                    int32_t dot = vx*nox[f] + vy*noy[f] + vz*noz[f];
                    if (dot <= 0) continue;

                    // 3) project & collect the 4 verts
                    int sxv[4], syv[4], vis = 0;
                    for (int i = 0; i < 4; i++) {
                        int v = face_verts[f][i];
                        int32_t wx = (((int32_t)bx + dx[v]) << FP_SHIFT) - px;
                        int32_t wy = (((int32_t)by + dy[v]) << FP_SHIFT) - p->y;
                        int32_t wz = (((int32_t)bz + dz[v]) << FP_SHIFT) - pz;
                        if (project_point(wx, wy, wz, p, proj_dist, &sxv[i], &syv[i]))
                            vis |= (1 << i);
                    }
                    if (!vis) continue;

                    // Optional: skip tiny or distant dots
                    if ((vis == 0b1111) && dot > 0) {
                        // Use center of face (average of verts)
                        int cx = (sxv[0] + sxv[1] + sxv[2] + sxv[3]) >> 2;
                        int cy = (syv[0] + syv[1] + syv[2] + syv[3]) >> 2;

                        int dx = cx - screenCX;
                        int dy = cy - screenCY;
                        int dist2 = dx * dx + dy * dy;

                        // Bias toward center (lower dist² is better)
                        if (dist2 < bestScore) {
                            selectedBlockX = bx;
                            selectedBlockY = by;
                            selectedBlockZ = bz;
                            selectedFace   = f;
                            bestScore = dist2;
                            for (int i = 0; i < 4; i++) {
                                bestFaceX[i] = sxv[i];
                                bestFaceY[i] = syv[i];
                            }
                            hasBestFace = true;
                        }
                    }

                    // 4) draw the 4 visible edges (wireframe)
                    gfx_SetColor(0);
                    for (int i = 0; i < 4; i++) {
                        int j = (i + 1) & 3;
                        if ((vis & (1<<i)) && (vis & (1<<j))) {
                            gfx_Line(sxv[i], syv[i],
                                     sxv[j], syv[j]);
                        }
                    }

                    // 5) colored‐fill triangles
                    int bt = chunk->blocks[bx][by][bz];
                    const int M = -40;  // your marginOfError
                    uint8_t cLight = block_getColor(bt, f, SHADE_LIGHT);
                    uint8_t cDark  = block_getColor(bt, f, SHADE_DARK);

                    // Triangle A: verts 0,1,2 (light)
                    if ((vis & 0b0111) == 0b0111) {
                        int x0 = sxv[0], y0 = syv[0];
                        int x1 = sxv[1], y1 = syv[1];
                        int x2 = sxv[2], y2 = syv[2];
                        // simple bounding-box
                        int minx = x0 < x1 ? x0 : x1, maxx = x0 > x1 ? x0 : x1;
                        minx = minx < x2 ? minx : x2;  maxx = maxx > x2 ? maxx : x2;
                        int miny = y0 < y1 ? y0 : y1, maxy = y0 > y1 ? y0 : y1;
                        miny = miny < y2 ? miny : y2;  maxy = maxy > y2 ? maxy : y2;
                        if (minx < SCREEN_W + M && maxx >= -M &&
                            miny < SCREEN_H + M && maxy >= -M)
                        {
                            gfx_SetColor(cLight);
                            if (opt_render_resolution == 1)
                                gfx_FillRectangle(
                                    (x0 < x1 ? x0 : x1),
                                    (y0 < y1 ? y0 : y1),
                                    (x0 > x1 ? x0 - x1 : x1 - x0),
                                    (y0 > y1 ? y0 - y1 : y1 - y0)
                                );
                            else if (opt_render_resolution == 2)
                                gfx_FillTriangle(x0, y0, x1, y1, x2, y2);
                        }
                    }

                    // Triangle B: verts 0,2,3 (dark)
                    if ((vis & 0b1101) == 0b1101) {
                        int x0 = sxv[0], y0 = syv[0];
                        int x2 = sxv[2], y2 = syv[2];
                        int x3 = sxv[3], y3 = syv[3];
                        int minx = x0 < x2 ? x0 : x2, maxx = x0 > x2 ? x0 : x2;
                        minx = minx < x3 ? minx : x3;  maxx = maxx > x3 ? maxx : x3;
                        int miny = y0 < y2 ? y0 : y2, maxy = y0 > y2 ? y0 : y2;
                        miny = miny < y3 ? miny : y3;  maxy = maxy > y3 ? maxy : y3;
                        if (minx < SCREEN_W + M && maxx >= -M &&
                            miny < SCREEN_H + M && maxy >= -M)
                        {
                            gfx_SetColor(cDark);
                            if (opt_render_resolution == 1)
                                gfx_FillRectangle(
                                    (x0 < x2 ? x0 : x2),
                                    (y0 < y2 ? y0 : y2),
                                    (x0 > x2 ? x0 - x2 : x2 - x0),
                                    (y0 > y2 ? y0 - y2 : y2 - y0)
                                );
                            else if (opt_render_resolution == 2)
                                gfx_FillTriangle(x0, y0, x2, y2, x3, y3);
                        }
                    }

                    // done this column
                    //break;
                }
            }
        }
    }
    // draw the cursor on the block in the middle of the screen if it's aligned
    if (hasBestFace) {
        gfx_SetColor(254); // white border
        for (int i = 0; i < 4; i++) {
            int j = (i + 1) & 3;
            gfx_Line(bestFaceX[i], bestFaceY[i],
                    bestFaceX[j], bestFaceY[j]);
        }
    }
    // crosshair with thickness 3
    gfx_SetColor(0);
    // Horizontal lines
    gfx_Line(SCREEN_W/2 - 5, SCREEN_H/2 - 1, SCREEN_W/2 + 5, SCREEN_H/2 - 1);
    gfx_Line(SCREEN_W/2 - 5, SCREEN_H/2,     SCREEN_W/2 + 5, SCREEN_H/2);
    gfx_Line(SCREEN_W/2 - 5, SCREEN_H/2 + 1, SCREEN_W/2 + 5, SCREEN_H/2 + 1);

    // Vertical lines
    gfx_Line(SCREEN_W/2 - 1, SCREEN_H/2 - 5, SCREEN_W/2 - 1, SCREEN_H/2 + 5);
    gfx_Line(SCREEN_W/2,     SCREEN_H/2 - 5, SCREEN_W/2,     SCREEN_H/2 + 5);
    gfx_Line(SCREEN_W/2 + 1, SCREEN_H/2 - 5, SCREEN_W/2 + 1, SCREEN_H/2 + 5);
}