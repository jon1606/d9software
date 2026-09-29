#include "d9_core/drive.hpp"

#include <gtest/gtest.h>

#include <limits>

#include "d9_core/config.hpp"

namespace d9 {
namespace {

constexpr float kMax = config::kMaxTrackSpeed;

TEST(ClampDrive, LeavesCommandsWithinLimitsUnchanged) {
  const DriveCommand out = clampDrive({0.5f * kMax, -0.25f * kMax});
  EXPECT_FLOAT_EQ(out.left, 0.5f * kMax);
  EXPECT_FLOAT_EQ(out.right, -0.25f * kMax);
}

TEST(ClampDrive, LimitsEachTrackIndependently) {
  const DriveCommand out = clampDrive({kMax + 1.0f, -kMax - 1.0f});
  EXPECT_FLOAT_EQ(out.left, kMax);
  EXPECT_FLOAT_EQ(out.right, -kMax);
}

TEST(ClampDrive, KeepsTheExactLimit) {
  const DriveCommand out = clampDrive({kMax, -kMax});
  EXPECT_FLOAT_EQ(out.left, kMax);
  EXPECT_FLOAT_EQ(out.right, -kMax);
}

TEST(ClampDrive, StopsBothTracksOnNaN) {
  // One garbage track alone would spin the D9 in place; stop both instead.
  const DriveCommand out = clampDrive({std::numeric_limits<float>::quiet_NaN(), 0.3f});
  EXPECT_FLOAT_EQ(out.left, 0.0f);
  EXPECT_FLOAT_EQ(out.right, 0.0f);
}

TEST(ClampDrive, StopsBothTracksOnInfinity) {
  const DriveCommand out = clampDrive({0.3f, -std::numeric_limits<float>::infinity()});
  EXPECT_FLOAT_EQ(out.left, 0.0f);
  EXPECT_FLOAT_EQ(out.right, 0.0f);
}

}  // namespace
}  // namespace d9
