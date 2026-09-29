#include "d9_core/geometry.hpp"

#include <cmath>

#include "d9_core/angles.hpp"

namespace d9 {
namespace {
constexpr float kRadToDeg = 57.29577951f;
}  // namespace

float distanceBetween(Point2 a, Point2 b) { return std::hypot(b.x - a.x, b.y - a.y); }

float headingFromPoints(Point2 from, Point2 to) {
  // atan2 can return -pi (e.g. for a -0.0 y difference); normalize keeps (-180, 180].
  return normalizeAngle(std::atan2(to.y - from.y, to.x - from.x) * kRadToDeg);
}

}  // namespace d9
