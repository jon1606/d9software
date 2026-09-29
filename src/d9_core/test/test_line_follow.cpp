#include "d9_core/line_follow.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

#include "d9_core/config.hpp"

namespace d9 {
namespace {

using config::kLineSearchTimeoutMs;
using config::kSearchTurnSpeed;

constexpr LineReading kLost{0.0f, false};

LineReading seen(float offset) { return {offset, true}; }

void expectStopped(const DriveCommand& cmd) {
  EXPECT_FLOAT_EQ(cmd.left, 0.0f);
  EXPECT_FLOAT_EQ(cmd.right, 0.0f);
}

void expectSameCommand(const DriveCommand& a, const DriveCommand& b) {
  EXPECT_FLOAT_EQ(a.left, b.left);
  EXPECT_FLOAT_EQ(a.right, b.right);
}

float forwardSpeed(const DriveCommand& cmd) { return (cmd.left + cmd.right) / 2.0f; }

// ---- lineSteer -------------------------------------------------------------

TEST(LineSteer, DrivesStraightAtCruiseWhenCentred) {
  const DriveCommand cmd = lineSteer(0.0f);
  EXPECT_FLOAT_EQ(cmd.left, config::kCruiseSpeed);
  EXPECT_FLOAT_EQ(cmd.right, config::kCruiseSpeed);
}

TEST(LineSteer, TurnsTowardTheLine) {
  const DriveCommand line_left = lineSteer(0.4f);
  EXPECT_GT(line_left.right, line_left.left);  // counter-clockwise, toward the line
  const DriveCommand line_right = lineSteer(-0.4f);
  EXPECT_GT(line_right.left, line_right.right);
}

TEST(LineSteer, IsMirrorSymmetric) {
  const DriveCommand a = lineSteer(0.6f);
  const DriveCommand b = lineSteer(-0.6f);
  EXPECT_FLOAT_EQ(a.left, b.right);
  EXPECT_FLOAT_EQ(a.right, b.left);
}

TEST(LineSteer, SlowsDownOnCurves) {
  EXPECT_LT(forwardSpeed(lineSteer(0.5f)), forwardSpeed(lineSteer(0.0f)));
  EXPECT_LT(forwardSpeed(lineSteer(1.0f)), forwardSpeed(lineSteer(0.5f)));
  EXPECT_FLOAT_EQ(forwardSpeed(lineSteer(1.0f)), config::kMinCurveSpeed);
}

TEST(LineSteer, TreatsOffsetsBeyondTheSensorAsTheEdge) {
  expectSameCommand(lineSteer(3.0f), lineSteer(1.0f));
  expectSameCommand(lineSteer(-3.0f), lineSteer(-1.0f));
}

TEST(LineSteer, StaysWithinTheMotorClamp) {
  for (float offset = -1.0f; offset <= 1.0f; offset += 0.125f) {
    const DriveCommand cmd = lineSteer(offset);
    EXPECT_LE(cmd.left, config::kMaxTrackSpeed) << offset;
    EXPECT_GE(cmd.left, -config::kMaxTrackSpeed) << offset;
    EXPECT_LE(cmd.right, config::kMaxTrackSpeed) << offset;
    EXPECT_GE(cmd.right, -config::kMaxTrackSpeed) << offset;
  }
}

TEST(LineSteer, StopsOnNaNOffset) {
  expectStopped(lineSteer(std::numeric_limits<float>::quiet_NaN()));
}

// ---- LineFollower ----------------------------------------------------------

TEST(LineFollower, StartsFollowingWithMotorsStopped) {
  LineFollower follower;
  EXPECT_EQ(follower.state(), LineFollowState::kFollowing);
  expectStopped(follower.command());
}

TEST(LineFollower, SteersByTheDetectedLine) {
  LineFollower follower;
  follower.update(100, seen(0.3f));
  EXPECT_EQ(follower.state(), LineFollowState::kFollowing);
  expectSameCommand(follower.command(), lineSteer(0.3f));
}

TEST(LineFollower, PivotsLeftWhenTheLineWasLastSeenLeft) {
  LineFollower follower;
  follower.update(100, seen(0.5f));
  follower.update(120, kLost);
  EXPECT_EQ(follower.state(), LineFollowState::kSearching);
  EXPECT_FLOAT_EQ(follower.command().left, -kSearchTurnSpeed);
  EXPECT_FLOAT_EQ(follower.command().right, kSearchTurnSpeed);
}

TEST(LineFollower, PivotsRightWhenTheLineWasLastSeenRight) {
  LineFollower follower;
  follower.update(100, seen(-0.5f));
  follower.update(120, kLost);
  EXPECT_EQ(follower.state(), LineFollowState::kSearching);
  EXPECT_FLOAT_EQ(follower.command().left, kSearchTurnSpeed);
  EXPECT_FLOAT_EQ(follower.command().right, -kSearchTurnSpeed);
}

TEST(LineFollower, ResumesFollowingWhenTheLineReappears) {
  LineFollower follower;
  follower.update(100, seen(0.5f));
  follower.update(120, kLost);
  follower.update(140, seen(0.8f));
  EXPECT_EQ(follower.state(), LineFollowState::kFollowing);
  expectSameCommand(follower.command(), lineSteer(0.8f));
}

TEST(LineFollower, KeepsSearchingUntilTheTimeout) {
  LineFollower follower;
  follower.update(100, seen(0.5f));
  follower.update(200, kLost);
  follower.update(200 + kLineSearchTimeoutMs - 1, kLost);
  EXPECT_EQ(follower.state(), LineFollowState::kSearching);
}

TEST(LineFollower, StopsWhenTheSearchTimesOut) {
  LineFollower follower;
  follower.update(100, seen(0.5f));
  follower.update(200, kLost);
  follower.update(200 + kLineSearchTimeoutMs, kLost);
  EXPECT_EQ(follower.state(), LineFollowState::kLost);
  expectStopped(follower.command());
}

TEST(LineFollower, StaysLostEvenIfTheLineReappears) {
  // Once stopped, the task layer decides what happens next; the D9 never restarts itself.
  LineFollower follower;
  follower.update(100, seen(0.5f));
  follower.update(200, kLost);
  follower.update(200 + kLineSearchTimeoutMs, kLost);
  follower.update(300 + kLineSearchTimeoutMs, seen(0.0f));
  EXPECT_EQ(follower.state(), LineFollowState::kLost);
  expectStopped(follower.command());
}

TEST(LineFollower, RestartsTheSearchTimerOnEachNewLoss) {
  LineFollower follower;
  follower.update(100, seen(0.5f));
  follower.update(200, kLost);
  follower.update(300, seen(0.5f));
  follower.update(400, kLost);
  follower.update(400 + kLineSearchTimeoutMs - 1, kLost);
  EXPECT_EQ(follower.state(), LineFollowState::kSearching);
}

TEST(LineFollower, TimesOutCorrectlyAcrossMillisWrap) {
  constexpr uint32_t kLostAt = 0xFFFFFF00u;  // millis() wraps to 0 ~49.7 days after boot
  LineFollower follower;
  follower.update(kLostAt - 10, seen(0.5f));
  follower.update(kLostAt, kLost);
  follower.update(kLostAt + kLineSearchTimeoutMs - 1, kLost);  // wrapped past zero
  EXPECT_EQ(follower.state(), LineFollowState::kSearching);
  follower.update(kLostAt + kLineSearchTimeoutMs, kLost);
  EXPECT_EQ(follower.state(), LineFollowState::kLost);
}

TEST(LineFollower, ResetStartsOverFromAnyState) {
  LineFollower follower;
  follower.update(100, seen(0.5f));
  follower.update(200, kLost);
  follower.update(200 + kLineSearchTimeoutMs, kLost);
  follower.reset();
  EXPECT_EQ(follower.state(), LineFollowState::kFollowing);
  expectStopped(follower.command());
  follower.update(5000, seen(-0.2f));
  expectSameCommand(follower.command(), lineSteer(-0.2f));
}

}  // namespace
}  // namespace d9
