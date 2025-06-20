#include "EaseInOutCubic.h"
#include <math.h>

float EaseInOutCubic(const float& x) { return x < 0.5f ? 4 * x * x * x : 1.0f - powf(-2.0f * x + 2, 3) / 2.0f; }