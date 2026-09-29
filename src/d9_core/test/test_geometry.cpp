#include "d9_core/geometry.hpp"

#include <gtest/gtest.h>

namespace d9 {
namespace {

constexpr float kEps = 1e-4f;

TEST(DistanceBetween, IsEuclidean) {
  EXPECT_NEAR(distanceBetween({0.0f, 0.0f}, {3.0f, 4.0f}), 5.0f, kEps);
  EXPECT_NEAR(distanceBetween({3.0f, 4.0f}, {0.0f, 0.0f}), 5.0f, kEps);
  EXPECT_NEAR(distanceBetween({-1.0f, 2.0f}, {2.0f, -2.0f}), 5.0f, kEps);
}

TEST(DistanceBetween, IsZeroForTheSamePoint) {
  EXPECT_NEAR(distanceBetween({7.0f, -3.0f}, {7.0f, -3.0f}), 0.0f, kEps);
}

TEST(HeadingFromPoints, FollowsTheAxesCounterClockwise) {
  EXPECT_NEAR(headingFromPoints({0.0f, 0.0f}, {10.0f, 0.0f}), 0.0f, kEps);
  EXPECT_NEAR(headingFromPoints({0.0f, 0.0f}, {0.0f, 10.0f}), 90.0f, kEps);
  EXPECT_NEAR(headingFromPoints({0.0f, 0.0f}, {-10.0f, 0.0f}), 180.0f, kEps);
  EXPECT_NEAR(headingFromPoints({0.0f, 0.0f}, {0.0f, -10.0f}), -90.0f, kEps);
}

TEST(HeadingFromPoints, HandlesDiagonals) {
  EXPECT_NEAR(headingFromPoints({1.0f, 1.0f}, {2.0f, 2.0f}), 45.0f, kEps);
  EXPECT_NEAR(headingFromPoints({0.0f, 0.0f}, {-1.0f, -1.0f}), -135.0f, kEps);
}

TEST(HeadingFromPoints, UsesTheDifferenceNotAbsolutePositions) {
  // Spec: heading = atan2(y2 - y1, x2 - x1) from two drone fixes ~20 cm apart.
  EXPECT_NEAR(headingFromPoints({100.0f, 50.0f}, {100.0f, 70.0f}), 90.0f, kEps);
}

TEST(HeadingFromPoints, ReportsMinusXAs180NeverMinus180) {
  // A -0.0 y difference makes atan2 return -pi; the result must still be in (-180, 180].
  EXPECT_NEAR(headingFromPoints({0.0f, 0.0f}, {-5.0f, -0.0f}), 180.0f, kEps);
}

TEST(HeadingFromPoints, IsZeroForIdenticalPoints) {
  // Undefined direction; callers must check the baseline before trusting a heading.
  EXPECT_NEAR(headingFromPoints({4.0f, 4.0f}, {4.0f, 4.0f}), 0.0f, kEps);
}

}  // namespace
}  // namespace d9
