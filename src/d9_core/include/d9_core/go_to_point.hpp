#pragma once

#include <cstdint>

#include "d9_core/arrival.hpp"
#include "d9_core/drive.hpp"
#include "d9_core/geometry.hpp"
#include "d9_core/line_follow.hpp"

namespace d9 {

enum class NavStatus : uint8_t {
  kIdle,       // no target: motors stopped
  kFollowing,  // on the line, heading for the target
  kSearching,  // line lost for now, pivoting to find it
  kArrived,    // within tolerance of the target (latched)
  kOvershot,   // passed the target without arriving (latched)
  kLineLost,   // line not found within the timeout (latched)
};

// FR-1 on the road (amendment A1): steer by the line, let drone fixes decide arrival.
// Every terminal status stops the motors and holds until start() or cancel().
class GoToPoint {
 public:
  void start(Point2 target);
  // The stop path: motors off immediately, from any status.
  void cancel();
  // Latest drone fix, already converted to the internal frame.
  void onPosition(Point2 position);
  void update(uint32_t now_ms, LineReading line);

  NavStatus status() const;
  DriveCommand command() const;

 private:
  void finish(NavStatus terminal);

  NavStatus status_ = NavStatus::kIdle;
  DriveCommand command_ = kStopDrive;
  LineFollower follower_;
  ArrivalMonitor arrival_;
};

}  // namespace d9
