#pragma once

// Tunable numbers for d9_core. Logic never hardcodes these.
// PROVISIONAL values are placeholders until the named owner confirms them; tests derive
// their cases from these constants, so retuning does not break them.

namespace d9::config {

// Safety cap on each track command, as a fraction of full speed.
// PROVISIONAL: our team, once the real motors are measured.
constexpr float kMaxTrackSpeed = 0.8f;

}  // namespace d9::config
