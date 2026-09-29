#include "d9_core/drive.hpp"

#include <algorithm>
#include <cmath>

#include "d9_core/config.hpp"

namespace d9 {

DriveCommand clampDrive(DriveCommand cmd) {
  if (!std::isfinite(cmd.left) || !std::isfinite(cmd.right)) {
    return kStopDrive;
  }
  constexpr float kMax = config::kMaxTrackSpeed;
  return {std::clamp(cmd.left, -kMax, kMax), std::clamp(cmd.right, -kMax, kMax)};
}

}  // namespace d9
