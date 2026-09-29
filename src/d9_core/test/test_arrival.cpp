#include "d9_core/arrival.hpp"

#include <gtest/gtest.h>

#include "d9_core/config.hpp"

namespace d9 {
namespace {

using config::kApproachRadiusCm;
using config::kApproachSpeedScale;
using config::kArriveToleranceCm;
using config::kOvershootMarginCm;

constexpr Point2 kTarget{0.0f, 0.0f};

// A drone fix `d` cm from the target.
Point2 fixAt(float d) { return {d, 0.0f}; }

ArrivalMonitor startedMonitor() {
  ArrivalMonitor monitor;
  monitor.start(kTarget);
  return monitor;
}

TEST(ArrivalMonitor, RunsAtFullSpeedBeforeTheFirstFix) {
  const ArrivalMonitor monitor = startedMonitor();
  EXPECT_EQ(monitor.state(), ArrivalState::kApproaching);
  EXPECT_FLOAT_EQ(monitor.speedScale(), 1.0f);
}

TEST(ArrivalMonitor, IsApproachingAtFullSpeedWhenFar) {
  ArrivalMonitor monitor = startedMonitor();
  monitor.onPosition(fixAt(kApproachRadiusCm + 50.0f));
  EXPECT_EQ(monitor.state(), ArrivalState::kApproaching);
  EXPECT_FLOAT_EQ(monitor.speedScale(), 1.0f);
}

TEST(ArrivalMonitor, SlowsDownInsideTheApproachRadius) {
  ArrivalMonitor monitor = startedMonitor();
  monitor.onPosition(fixAt(kApproachRadiusCm));
  EXPECT_EQ(monitor.state(), ArrivalState::kNear);
  EXPECT_FLOAT_EQ(monitor.speedScale(), kApproachSpeedScale);
}

TEST(ArrivalMonitor, ArrivesWithinTolerance) {
  ArrivalMonitor monitor = startedMonitor();
  monitor.onPosition(fixAt(kArriveToleranceCm));
  EXPECT_EQ(monitor.state(), ArrivalState::kArrived);
  EXPECT_FLOAT_EQ(monitor.speedScale(), 0.0f);
}

TEST(ArrivalMonitor, StaysArrivedWhenLaterFixesAreNoisy) {
  ArrivalMonitor monitor = startedMonitor();
  monitor.onPosition(fixAt(kArriveToleranceCm - 1.0f));
  monitor.onPosition(fixAt(kApproachRadiusCm + 50.0f));
  EXPECT_EQ(monitor.state(), ArrivalState::kArrived);
  EXPECT_FLOAT_EQ(monitor.speedScale(), 0.0f);
}

TEST(ArrivalMonitor, DetectsOvershootAfterTheClosestApproach) {
  // On a loop road, missing the target costs a whole lap: stop as soon as it is passed.
  ArrivalMonitor monitor = startedMonitor();
  const float closest = kArriveToleranceCm + 5.0f;
  monitor.onPosition(fixAt(kApproachRadiusCm - 5.0f));
  monitor.onPosition(fixAt(closest));
  monitor.onPosition(fixAt(closest + kOvershootMarginCm + 1.0f));
  EXPECT_EQ(monitor.state(), ArrivalState::kOvershot);
  EXPECT_FLOAT_EQ(monitor.speedScale(), 0.0f);
}

TEST(ArrivalMonitor, IgnoresFixNoiseSmallerThanTheMargin) {
  ArrivalMonitor monitor = startedMonitor();
  const float closest = kArriveToleranceCm + 5.0f;
  monitor.onPosition(fixAt(closest));
  monitor.onPosition(fixAt(closest + kOvershootMarginCm - 1.0f));
  EXPECT_EQ(monitor.state(), ArrivalState::kNear);
}

TEST(ArrivalMonitor, StaysOvershotEvenIfLaterFixesReachTheTarget) {
  ArrivalMonitor monitor = startedMonitor();
  const float closest = kArriveToleranceCm + 5.0f;
  monitor.onPosition(fixAt(closest));
  monitor.onPosition(fixAt(closest + kOvershootMarginCm + 1.0f));
  monitor.onPosition(fixAt(0.0f));
  EXPECT_EQ(monitor.state(), ArrivalState::kOvershot);
}

TEST(ArrivalMonitor, MovingAwayBeforeGettingNearIsNotAnOvershoot) {
  // The road curves: distance may grow for a while long before the target is reached.
  ArrivalMonitor monitor = startedMonitor();
  monitor.onPosition(fixAt(kApproachRadiusCm + 40.0f));
  monitor.onPosition(fixAt(kApproachRadiusCm + 10.0f));
  monitor.onPosition(fixAt(kApproachRadiusCm + 30.0f));
  EXPECT_EQ(monitor.state(), ArrivalState::kApproaching);
  EXPECT_FLOAT_EQ(monitor.speedScale(), 1.0f);
}

TEST(ArrivalMonitor, StartForgetsThePreviousTarget) {
  ArrivalMonitor monitor = startedMonitor();
  monitor.onPosition(fixAt(kArriveToleranceCm + 5.0f));  // closest to the old target
  monitor.start({500.0f, 0.0f});
  monitor.onPosition({500.0f - kApproachRadiusCm - 50.0f, 0.0f});
  EXPECT_EQ(monitor.state(), ArrivalState::kApproaching);
  EXPECT_FLOAT_EQ(monitor.speedScale(), 1.0f);
}

}  // namespace
}  // namespace d9
