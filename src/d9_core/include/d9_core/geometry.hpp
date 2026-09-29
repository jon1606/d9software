#pragma once

namespace d9 {

// A position in the internal frame: centimetres, 0 deg along +x, counter-clockwise positive.
struct Point2 {
  float x;
  float y;
};

float distanceBetween(Point2 a, Point2 b);

// Direction of travel from `from` to `to`, in degrees, in (-180, 180].
// Identical points give 0: the direction is undefined, so callers must check that the
// two points are far enough apart before trusting the result.
float headingFromPoints(Point2 from, Point2 to);

}  // namespace d9
