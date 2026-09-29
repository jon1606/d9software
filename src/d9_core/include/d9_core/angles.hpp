#pragma once

namespace d9 {

// Wraps any angle in degrees into (-180, 180]. Compare angles only after this.
float normalizeAngle(float deg);

// Signed shortest turn from `current` to `target`, in degrees, in (-180, 180].
// Positive means turn counter-clockwise (left).
float angleError(float target_deg, float current_deg);

}  // namespace d9
