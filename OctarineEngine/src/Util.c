#include "VK2D/VK2D.h"

#include "oct/Util.h"

OCTARINE_API float oct_Random(float min, float max) {
    return vk2dRandom(min, max);
}
