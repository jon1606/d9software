#include "d9_core/arrival.hpp"

#include <algorithm>

#include "d9_core/config.hpp"

namespace d9 {

void ArrivalMonitor::start(Point2 target) {
  *this = ArrivalMonitor{};
  target_ = target;
}

void ArrivalMonitor::onPosition(Point2 position) {
  if (state_ == ArrivalState::kArrived || state_ == ArrivalState::kOvershot) {
    return;
  }
  const float distance = distanceBetween(position, target_);
  closest_cm_ = std::min(closest_cm_, distance);
  // Growth only counts once the D9 has been near: far away, the curving road can
  // legitimately lead away from the target for a while.
  const bool was_near = closest_cm_ <= config::kApproachRadiusCm;
  if (distance <= config::kArriveToleranceCm) {
    state_ = ArrivalState::kArrived;
  } else if (was_near && distance > closest_cm_ + config::kOvershootMarginCm) {
    state_ = ArrivalState::kOvershot;
  } else if (distance <= config::kApproachRadiusCm) {
    state_ = ArrivalState::kNear;
  } else {
    state_ = ArrivalState::kApproaching;
  }
}

ArrivalState ArrivalMonitor::state() const { return state_; }

float ArrivalMonitor::speedScale() const {
  switch (state_) {
    case ArrivalState::kApproaching:
      return 1.0f;
    case ArrivalState::kNear:
      return config::kApproachSpeedScale;
    case ArrivalState::kArrived:
    case ArrivalState::kOvershot:
      break;
  }
  return 0.0f;
}

}  // namespace d9
