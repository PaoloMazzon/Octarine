/// \brief Tilemap
#pragma once
#include "oct/Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Creates a new tilemap you can use to draw many tiles and check collisions quickly
OCTARINE_API Oct_Tilemap oct_CreateTilemap(Oct_Texture tex, int32_t width, int32_t height, Oct_Vec2 cellSize);

/// \brief Returns the value of a specific cell, returns 0 if that cell is out of range of the map
OCTARINE_API int32_t oct_GetTilemap(Oct_Tilemap map, int32_t x, int32_t y);

/// \brief Sets a value in the tilemap, does nothing if out of range
OCTARINE_API void oct_SetTilemap(Oct_Tilemap map, int32_t x, int32_t y, int32_t val);

/// \brief Returns the width in cells of a tilemap
OCTARINE_API int32_t oct_TilemapWidth(Oct_Tilemap map);

/// \brief Returns the height in cells of a tilemap
OCTARINE_API int32_t oct_TilemapHeight(Oct_Tilemap map);

/// \brief Returns the width in pixels of a cell in a tilemap
OCTARINE_API float oct_TilemapCellWidth(Oct_Tilemap map);

/// \brief Returns the height in pixels of a cell in a tilemap
OCTARINE_API float oct_TilemapCellHeight(Oct_Tilemap map);

/// \brief Returns the internal texture of a tilemap
OCTARINE_API Oct_Texture oct_TilemapTexture(Oct_Tilemap map);

/// \brief Checks if any part of a rectangle is colliding with the tilemap (it intersects with a non-zero cell)
OCTARINE_API Oct_Bool oct_TilemapCollision(Oct_Tilemap map, Oct_Rectangle *rect);

/// \brief Draws a complete tilemap using the internal texture
OCTARINE_API void oct_TilemapDraw(Oct_Tilemap map);

/// \brief Draws part of a tilemap
OCTARINE_API void oct_TilemapDrawPart(Oct_Tilemap map, int32_t x, int32_t y, int32_t width, int32_t height);

/// \brief Frees a tilemap from memory
OCTARINE_API void oct_DestroyTilemap(Oct_Tilemap map);

#ifdef __cplusplus
};
#endif
