#pragma once

#include <cstdint>

#include "d9_core/drive.hpp"

namespace d9 {

// Sensor-agnostic view of the road line under the D9 (amendment A1).
// offset: -1..1 across the sensor, positive = line is LEFT of centre, so a positive
// offset means "turn counter-clockwise", like a positive angle. Ignored if !detected.
struct LineReading {
  float offset;
  bool detected;
};

// Steering for a visible line: turn toward it, and slow down as it moves off-centre,
// which on this road means a curve. Offsets beyond +/-1 count as the sensor edge.
DriveCommand lineSteer(float offset);

enum class LineFollowState : uint8_t {
  kFollowing,  // line visible (or nothing read yet)
  kSearching,  // line just lost: pivot toward where it was last seen
  kLost,       // not found within the timeout: motors stopped until reset()
};

class LineFollower {
 public:
  void update(uint32_t now_ms, LineReading reading);
  void reset();

  LineFollowState state() const;
  DriveCommand command() const;

 private:
  LineFollowState state_ = LineFollowState::kFollowing;
  DriveCommand command_ = kStopDrive;
  float last_offset_ = 0.0f;
  uint32_t lost_since_ms_ = 0;
};

}  // namespace d9
