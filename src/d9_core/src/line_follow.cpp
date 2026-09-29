#include "d9_core/line_follow.hpp"

#include <algorithm>
#include <cmath>

#include "d9_core/config.hpp"

namespace d9 {

DriveCommand lineSteer(float offset) {
  // NaN passes through std::clamp and is then stopped by clampDrive.
  const float o = std::clamp(offset, -1.0f, 1.0f);
  // Forward speed falls linearly from cruise (centred) to the curve minimum (at the edge).
  const float forward =
      config::kCruiseSpeed - (config::kCruiseSpeed - config::kMinCurveSpeed) * std::fabs(o);
  const float steer = config::kLineSteerGain * o;
  return clampDrive({forward - steer, forward + steer});
}

void LineFollower::update(uint32_t now_ms, LineReading reading) {
  if (state_ == LineFollowState::kLost) {
    return;
  }
  if (reading.detected) {
    state_ = LineFollowState::kFollowing;
    last_offset_ = reading.offset;
    command_ = lineSteer(reading.offset);
    return;
  }
  if (state_ == LineFollowState::kFollowing) {
    state_ = LineFollowState::kSearching;
    lost_since_ms_ = now_ms;
  }
  // Unsigned subtraction stays correct when millis() wraps around.
  if (now_ms - lost_since_ms_ >= config::kLineSearchTimeoutMs) {
    state_ = LineFollowState::kLost;
    command_ = kStopDrive;
    return;
  }
  // A sharp curve takes the line off the side it was last seen on; ties pivot left.
  const float turn = last_offset_ >= 0.0f ? config::kSearchTurnSpeed : -config::kSearchTurnSpeed;
  command_ = clampDrive({-turn, turn});
}

void LineFollower::reset() { *this = LineFollower{}; }

LineFollowState LineFollower::state() const { return state_; }

DriveCommand LineFollower::command() const { return command_; }

}  // namespace d9
