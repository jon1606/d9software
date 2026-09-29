#include "d9_core/go_to_point.hpp"

namespace d9 {
namespace {

bool isActive(NavStatus status) {
  return status == NavStatus::kFollowing || status == NavStatus::kSearching;
}

DriveCommand scaled(DriveCommand cmd, float factor) {
  return {cmd.left * factor, cmd.right * factor};
}

}  // namespace

void GoToPoint::start(Point2 target) {
  *this = GoToPoint{};
  arrival_.start(target);
  status_ = NavStatus::kFollowing;
}

void GoToPoint::cancel() { *this = GoToPoint{}; }

void GoToPoint::onPosition(Point2 position) {
  if (!isActive(status_)) {
    return;
  }
  arrival_.onPosition(position);
  // Stop here rather than on the next update(): arrival must not wait for a line reading.
  if (arrival_.state() == ArrivalState::kArrived) {
    finish(NavStatus::kArrived);
  } else if (arrival_.state() == ArrivalState::kOvershot) {
    finish(NavStatus::kOvershot);
  }
}

void GoToPoint::update(uint32_t now_ms, LineReading line) {
  if (!isActive(status_)) {
    command_ = kStopDrive;
    return;
  }
  follower_.update(now_ms, line);
  switch (follower_.state()) {
    case LineFollowState::kLost:
      finish(NavStatus::kLineLost);
      return;
    case LineFollowState::kSearching:
      // Not slowed near the target: a scaled-down pivot may not turn a tracked chassis.
      status_ = NavStatus::kSearching;
      command_ = follower_.command();
      return;
    case LineFollowState::kFollowing:
      status_ = NavStatus::kFollowing;
      command_ = clampDrive(scaled(follower_.command(), arrival_.speedScale()));
      return;
  }
}

NavStatus GoToPoint::status() const { return status_; }

DriveCommand GoToPoint::command() const { return command_; }

void GoToPoint::finish(NavStatus terminal) {
  status_ = terminal;
  command_ = kStopDrive;
}

}  // namespace d9
