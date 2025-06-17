#include <mimalloc.h>
#include <math.h>
#include "oct/Opaque.h"
#include "oct/Tilemap.h"
#include "oct/Validation.h"
#include "oct/Util.h"
#include "oct/Drawing.h"
#include "oct/CommandBuffer.h"
#include "oct/Assets.h"

#define GRID_LOC(tmap, x, y) tmap->grid[(y * tmap->width) + x]

OCTARINE_API Oct_Tilemap oct_CreateTilemap(Oct_Texture tex, int32_t width, int32_t height, Oct_Vec2 cellSize) {
    Oct_Tilemap map = mi_malloc(sizeof(struct Oct_Tilemap_t));
    if (!map)
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate tilemap");

    map->grid = mi_zalloc(width * height * sizeof(int32_t));
    if (!map->grid)
        oct_Raise(OCT_STATUS_OUT_OF_MEMORY, true, "Failed to allocate tilemap grid");

    map->cellSize[0] = cellSize[0];
    map->cellSize[1] = cellSize[1];
    map->width = width;
    map->height = height;
    map->tex = tex;
    map->texSize[0] = oct_TextureWidth(tex);
    map->texSize[1] = oct_TextureHeight(tex);

    return map;
}

OCTARINE_API int32_t oct_GetTilemap(Oct_Tilemap map, int32_t x, int32_t y) {
    if (x < map->width && x >= 0 && y < map->height && y >= 0)
        return GRID_LOC(map, x, y);
    return 0;
}

OCTARINE_API void oct_SetTilemap(Oct_Tilemap map, int32_t x, int32_t y, int32_t val) {
    if (x < map->width && x >= 0 && y < map->height && y >= 0)
        GRID_LOC(map, x, y) = val;
}

OCTARINE_API int32_t oct_TilemapWidth(Oct_Tilemap map) {
    return map->width;
}

OCTARINE_API int32_t oct_TilemapHeight(Oct_Tilemap map) {
    return map->height;
}

OCTARINE_API float oct_TilemapCellWidth(Oct_Tilemap map) {
    return map->cellSize[0];
}

OCTARINE_API float oct_TilemapCellHeight(Oct_Tilemap map) {
    return map->cellSize[1];
}

OCTARINE_API Oct_Texture oct_TilemapTexture(Oct_Tilemap map) {
    return map->tex;
}

OCTARINE_API Oct_Bool oct_TilemapCollision(Oct_Tilemap map, Oct_Rectangle *rect) {
    // TODO: Convert this from Wren to C lol
    /*if (hitbox.no_hit) {
        return false
    }
    var bb = hitbox.bounding_box(x, y)
    x = x - hitbox.x_offset
    y = y - hitbox.y_offset
    // this is to account for bounding boxes that end on a new spot and shouldn't
    bb[2] = bb[2] % _w == 0 ? bb[2] - 0.1 : bb[2]
    bb[3] = bb[3] % _h == 0 ? bb[3] - 0.1 : bb[3]
    var vertices_wide = 1 + ((bb[2] - bb[0]) / _w).ceil
    var vertices_tall = 1 + ((bb[3] - bb[1]) / _h).ceil
    var vertex_x = x
    var vertex_y = y

    for (y_index in 0..(vertices_tall - 1)) {
        vertex_x = x
        for (x_index in 0..(vertices_wide - 1)) {
            if (this[(vertex_x / _w).floor, (vertex_y / _h).floor] != 0) {
                return true
            }
            vertex_x = x_index == vertices_wide - 2 ? bb[2] : vertex_x + _w
        }
        vertex_y = y_index == vertices_tall - 2 ? bb[3] : vertex_y + _h
    }*/

    return false;
}

OCTARINE_API void oct_TilemapDraw(Oct_Tilemap map) {
    oct_TilemapDrawPart(map, 0, 0, map->width, map->height);
}

OCTARINE_API void oct_TilemapDrawPart(Oct_Tilemap map, int32_t x, int32_t y, int32_t width, int32_t height) {
    const int32_t minY = oct_Clampi(0, map->height, y);
    const int32_t maxY = oct_Clampi(0, map->height, y + height);
    const int32_t minX = oct_Clampi(0, map->width, x);
    const int32_t maxX = oct_Clampi(0, map->width, x + width);
    for (int32_t yIt = minY; yIt < maxY; yIt++) {
        for (int32_t xIt = minX; xIt < maxX; xIt++) {
            int32_t cell = GRID_LOC(map, xIt, yIt);
            if (cell == 0) continue;
            cell--;
            Oct_DrawCommand cmd = {
                    .type = OCT_DRAW_COMMAND_TYPE_TEXTURE,
                    .colour = {1, 1, 1, 1},
                    .Texture = {
                            .texture = map->tex,
                            .viewport = {
                                    .size = {
                                            map->cellSize[0],
                                            map->cellSize[1],
                                    },
                                    .position = {
                                            (int32_t)(cell * map->cellSize[0]) % (int32_t)(map->texSize[0]),
                                            (int32_t)(cell * map->cellSize[0]) / (int32_t)(map->texSize[0]) * map->cellSize[1]
                                    }
                                    },
                            .position = {
                                    xIt * map->cellSize[0],
                                    yIt * map->cellSize[1],
                                    },
                            .scale = {1, 1},
                    }
            };
            oct_Draw(&cmd);
        }
    }
}

OCTARINE_API void oct_DestroyTilemap(Oct_Tilemap map) {
    if (!map) return;
    mi_free(map->grid);
    mi_free(map);
}
