#include <math.h>
#include <stdio.h>
#include "VK2D/VK2D.h"
#include "oct/Subsystems.h"
#include "oct/Util.h"
#include "oct/Allocators.h"

OCTARINE_API float oct_Random(float min, float max) {
    return vk2dRandom(min, max);
}

OCTARINE_API Oct_Bool oct_AABB(Oct_Rectangle *r1, Oct_Rectangle *r2) {
    SDL_FRect sr1 = {
            .x = r1->position[0],
            .y = r1->position[1],
            .w = r1->position[0],
            .h = r1->position[1],
    };
    SDL_FRect sr2 = {
            .x = r2->position[0],
            .y = r2->position[1],
            .w = r2->position[0],
            .h = r2->position[1],
    };
    return SDL_HasRectIntersectionFloat(&sr1, &sr2);
}

OCTARINE_API float oct_PointDistance(Oct_Vec2 p1, Oct_Vec2 p2) {
    return sqrtf(powf(p2[1] - p1[1], 2) + powf(p2[0] - p1[0], 2));
}

OCTARINE_API float oct_PointAngle(Oct_Vec2 p1, Oct_Vec2 p2) {
    return atan2f(p2[0] - p1[0], p2[1] - p1[1]) - (VK2D_PI / 2);
}

OCTARINE_API float oct_Lerp(float min, float max, float val) {
    return ((max - min) * val) + min;
}

OCTARINE_API float oct_Sirp(float min, float max, float val) {
    const float m = (sinf(VK2D_PI * (val - 0.5)) / 2) + 0.5;
    return ((max - min) * m) + min;
}

OCTARINE_API float oct_Clamp(float min, float max, float val) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

OCTARINE_API double oct_Clampd(double min, double max, double val) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

OCTARINE_API int32_t oct_Clampi(int32_t min, int32_t max, int32_t val) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

OCTARINE_API Oct_Bool oct_FileExists(const char *filename) {
    FILE *f = fopen(filename, "rw");
    if (f)
        fclose(f);
    return f != null;
}

OCTARINE_API void *oct_ReadFile(const char *filename, Oct_Allocator allocator, uint32_t *size) {
    FILE* file = fopen(filename, "rb");
    unsigned char *buffer = null;
    *size = 0;

    if (file != null) {
        // Find file size - this technically not portable but idgaf
        fseek(file, 0, SEEK_END);
        *size = ftell(file);
        rewind(file);
        buffer = oct_Malloc(allocator, *size);

        if (buffer != null) {
            fread(buffer, 1, *size, file);
        }
        fclose(file);
    }
    return buffer;
}

OCTARINE_API Oct_Bool oct_WriteFile(const char *filename, void *data, uint32_t size) {
    FILE *f = fopen(filename, "wb");
    if (f) {
        fwrite(data, size, 1, f);
        fclose(f);
        return true;
    }
    return false;
}
