#include "d9_core/angles.hpp"

#include <gtest/gtest.h>

namespace d9 {
namespace {

constexpr float kEps = 1e-4f;

TEST(NormalizeAngle, KeepsInRangeValuesUnchanged) {
  EXPECT_NEAR(normalizeAngle(0.0f), 0.0f, kEps);
  EXPECT_NEAR(normalizeAngle(90.0f), 90.0f, kEps);
  EXPECT_NEAR(normalizeAngle(-179.5f), -179.5f, kEps);
}

TEST(NormalizeAngle, MapsBothEndsTo180) {
  EXPECT_NEAR(normalizeAngle(180.0f), 180.0f, kEps);
  EXPECT_NEAR(normalizeAngle(-180.0f), 180.0f, kEps);
}

TEST(NormalizeAngle, WrapsValuesJustOutsideRange) {
  EXPECT_NEAR(normalizeAngle(190.0f), -170.0f, kEps);
  EXPECT_NEAR(normalizeAngle(-190.0f), 170.0f, kEps);
}

TEST(NormalizeAngle, WrapsMultipleTurns) {
  EXPECT_NEAR(normalizeAngle(360.0f), 0.0f, kEps);
  EXPECT_NEAR(normalizeAngle(540.0f), 180.0f, kEps);
  EXPECT_NEAR(normalizeAngle(-540.0f), 180.0f, kEps);
  EXPECT_NEAR(normalizeAngle(725.0f), 5.0f, kEps);
}

TEST(AngleError, IsTargetMinusCurrentForSmallAngles) {
  EXPECT_NEAR(angleError(30.0f, 10.0f), 20.0f, kEps);   // target is CCW: turn left
  EXPECT_NEAR(angleError(10.0f, 30.0f), -20.0f, kEps);  // target is CW: turn right
}

TEST(AngleError, TakesTheShortWayAcrossTheWrap) {
  EXPECT_NEAR(angleError(-170.0f, 170.0f), 20.0f, kEps);
  EXPECT_NEAR(angleError(170.0f, -170.0f), -20.0f, kEps);
}

}  // namespace
}  // namespace d9
