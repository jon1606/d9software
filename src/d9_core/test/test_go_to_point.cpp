#include "d9_core/go_to_point.hpp"

#include <gtest/gtest.h>

#include "d9_core/config.hpp"

namespace d9 {
namespace {

using config::kApproachRadiusCm;
using config::kApproachSpeedScale;
using config::kArriveToleranceCm;
using config::kLineSearchTimeoutMs;
using config::kOvershootMarginCm;

constexpr Point2 kTarget{200.0f, 80.0f};
constexpr LineReading kNoLine{0.0f, false};

LineReading seen(float offset) { return {offset, true}; }

// A drone fix `d` cm from the target, along +x.
Point2 fixAt(float d) { return {kTarget.x + d, kTarget.y}; }

void expectStopped(const DriveCommand& cmd) {
  EXPECT_FLOAT_EQ(cmd.left, 0.0f);
  EXPECT_FLOAT_EQ(cmd.right, 0.0f);
}

// Drives a fresh GoToPoint into `wanted` through its public API only.
GoToPoint inStatus(NavStatus wanted) {
  GoToPoint nav;
  if (wanted == NavStatus::kIdle) return nav;
  nav.start(kTarget);
  nav.onPosition(fixAt(kApproachRadiusCm + 100.0f));
  nav.update(0, seen(0.2f));
  switch (wanted) {
    case NavStatus::kSearching:
      nav.update(10, kNoLine);
      break;
    case NavStatus::kLineLost:
      nav.update(10, kNoLine);
      nav.update(10 + kLineSearchTimeoutMs, kNoLine);
      break;
    case NavStatus::kArrived:
      nav.onPosition(fixAt(0.0f));
      break;
    case NavStatus::kOvershot:
      nav.onPosition(fixAt(kArriveToleranceCm + 5.0f));
      nav.onPosition(fixAt(kArriveToleranceCm + 5.0f + kOvershootMarginCm + 1.0f));
      break;
    default:
      break;
  }
  return nav;
}

TEST(GoToPoint, IsIdleAndStoppedBeforeStart) {
  GoToPoint nav;
  EXPECT_EQ(nav.status(), NavStatus::kIdle);
  expectStopped(nav.command());
}

TEST(GoToPoint, IgnoresTheLineWhileIdle) {
  GoToPoint nav;
  nav.update(100, seen(0.2f));
  EXPECT_EQ(nav.status(), NavStatus::kIdle);
  expectStopped(nav.command());
}

TEST(GoToPoint, StartsFollowingButWaitsForTheFirstUpdateToMove) {
  GoToPoint nav;
  nav.start(kTarget);
  EXPECT_EQ(nav.status(), NavStatus::kFollowing);
  expectStopped(nav.command());
}

TEST(GoToPoint, FollowsTheLineAtFullSpeedWhenFar) {
  GoToPoint nav;
  nav.start(kTarget);
  nav.onPosition(fixAt(kApproachRadiusCm + 100.0f));
  nav.update(100, seen(0.2f));
  EXPECT_EQ(nav.status(), NavStatus::kFollowing);
  EXPECT_FLOAT_EQ(nav.command().left, lineSteer(0.2f).left);
  EXPECT_FLOAT_EQ(nav.command().right, lineSteer(0.2f).right);
}

TEST(GoToPoint, SlowsDownNearTheTarget) {
  GoToPoint nav;
  nav.start(kTarget);
  nav.onPosition(fixAt(kApproachRadiusCm - 1.0f));
  nav.update(100, seen(0.2f));
  EXPECT_EQ(nav.status(), NavStatus::kFollowing);
  EXPECT_FLOAT_EQ(nav.command().left, lineSteer(0.2f).left * kApproachSpeedScale);
  EXPECT_FLOAT_EQ(nav.command().right, lineSteer(0.2f).right * kApproachSpeedScale);
}

TEST(GoToPoint, StopsAsSoonAsAFixSaysArrived) {
  // No update() needed: the stop must not wait for the next line reading.
  GoToPoint nav = inStatus(NavStatus::kFollowing);
  nav.onPosition(fixAt(kArriveToleranceCm - 1.0f));
  EXPECT_EQ(nav.status(), NavStatus::kArrived);
  expectStopped(nav.command());
}

TEST(GoToPoint, StaysStoppedAfterArrivingWhileTheLineIsStillSeen) {
  GoToPoint nav = inStatus(NavStatus::kArrived);
  nav.update(500, seen(0.0f));
  EXPECT_EQ(nav.status(), NavStatus::kArrived);
  expectStopped(nav.command());
}

TEST(GoToPoint, StopsWhenTheTargetIsOvershot) {
  GoToPoint nav = inStatus(NavStatus::kOvershot);
  EXPECT_EQ(nav.status(), NavStatus::kOvershot);
  expectStopped(nav.command());
  nav.update(500, seen(0.0f));
  expectStopped(nav.command());
}

TEST(GoToPoint, PivotsAtFullSearchSpeedWhileTheLineIsLost) {
  GoToPoint nav;
  nav.start(kTarget);
  nav.onPosition(fixAt(kApproachRadiusCm - 1.0f));  // near: search must not be slowed
  nav.update(0, seen(0.5f));
  nav.update(10, kNoLine);
  EXPECT_EQ(nav.status(), NavStatus::kSearching);
  EXPECT_FLOAT_EQ(nav.command().left, -config::kSearchTurnSpeed);
  EXPECT_FLOAT_EQ(nav.command().right, config::kSearchTurnSpeed);
}

TEST(GoToPoint, StopsWhenTheLineIsLostForGood) {
  GoToPoint nav = inStatus(NavStatus::kLineLost);
  EXPECT_EQ(nav.status(), NavStatus::kLineLost);
  expectStopped(nav.command());
}

TEST(GoToPoint, CancelStopsFromEveryStatus) {
  for (NavStatus status : {NavStatus::kIdle, NavStatus::kFollowing, NavStatus::kSearching,
                           NavStatus::kArrived, NavStatus::kOvershot, NavStatus::kLineLost}) {
    GoToPoint nav = inStatus(status);
    ASSERT_EQ(nav.status(), status);
    nav.cancel();
    EXPECT_EQ(nav.status(), NavStatus::kIdle) << static_cast<int>(status);
    expectStopped(nav.command());
    nav.update(1000, seen(0.2f));  // and it stays stopped afterwards
    expectStopped(nav.command());
  }
}

TEST(GoToPoint, StartAfterArrivingHeadsForTheNewTarget) {
  GoToPoint nav = inStatus(NavStatus::kArrived);
  nav.start({kTarget.x + 500.0f, kTarget.y});
  nav.onPosition(kTarget);  // far from the new target
  nav.update(600, seen(0.0f));
  EXPECT_EQ(nav.status(), NavStatus::kFollowing);
  EXPECT_FLOAT_EQ(nav.command().left, lineSteer(0.0f).left);
}

}  // namespace
}  // namespace d9
