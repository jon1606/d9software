#include "d9_core/angles.hpp"

#include <cmath>

namespace d9 {

float normalizeAngle(float deg) {
  // fmod keeps the sign of `deg`, so the result is in (-360, 360) before the shift.
  float wrapped = std::fmod(deg, 360.0f);
  if (wrapped <= -180.0f) {
    wrapped += 360.0f;
  } else if (wrapped > 180.0f) {
    wrapped -= 360.0f;
  }
  return wrapped;
}

float angleError(float target_deg, float current_deg) {
  return normalizeAngle(target_deg - current_deg);
}

}  // namespace d9
