#pragma once

// Tunable numbers for d9_core. Logic never hardcodes these.
// PROVISIONAL values are placeholders until the named owner confirms them; tests derive
// their cases from these constants, so retuning does not break them.

#include <cstdint>

namespace d9::config {

// Safety cap on each track command, as a fraction of full speed.
// PROVISIONAL: our team, once the real motors are measured.
constexpr float kMaxTrackSpeed = 0.8f;

// Line following (amendment A1). PROVISIONAL: our team, tuned in sim (M2), then on the road.
constexpr float kCruiseSpeed = 0.5f;             // forward speed with the line centred
constexpr float kMinCurveSpeed = 0.25f;          // forward speed with the line at the edge
constexpr float kLineSteerGain = 0.5f;           // per-track speed change at full offset
constexpr float kSearchTurnSpeed = 0.35f;        // pivot speed while the line is lost
constexpr uint32_t kLineSearchTimeoutMs = 1500;  // stop if the line is not found by then

}  // namespace d9::config
