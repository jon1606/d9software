#pragma once

#include <cstdint>
#include <limits>

#include "d9_core/geometry.hpp"

namespace d9 {

enum class ArrivalState : uint8_t {
  kApproaching,  // outside the approach radius: full speed
  kNear,         // inside the approach radius: slow down
  kArrived,      // within tolerance (latched)
  kOvershot,     // passed the closest point without arriving (latched)
};

// Decides from drone fixes when the D9 has reached a target on the road (amendment A1).
// It only judges distance; steering comes from the line follower.
class ArrivalMonitor {
 public:
  void start(Point2 target);
  void onPosition(Point2 position);

  ArrivalState state() const;
  // Factor for the forward command: 1 far away, reduced when near, 0 once done.
  float speedScale() const;

 private:
  Point2 target_{0.0f, 0.0f};
  ArrivalState state_ = ArrivalState::kApproaching;
  float closest_cm_ = std::numeric_limits<float>::max();
};

}  // namespace d9
