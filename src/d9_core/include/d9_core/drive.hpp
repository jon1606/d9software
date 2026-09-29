#pragma once

namespace d9 {

// Track speeds as fractions of full speed, positive = forward.
// left > right turns clockwise (right), right > left turns counter-clockwise (left).
struct DriveCommand {
  float left;
  float right;
};

constexpr DriveCommand kStopDrive{0.0f, 0.0f};

// Every command leaving d9_core passes through here: each track is limited to
// +/- config::kMaxTrackSpeed, and a non-finite value on either track stops both.
DriveCommand clampDrive(DriveCommand cmd);

}  // namespace d9
