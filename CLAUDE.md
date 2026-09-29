# Mini D9 — Autonomous Bulldozer Model

Software for a ~25×15 cm tracked D9 model, one of five vehicles in a multi-team system.
The D9 clears obstacles at server-given coordinates, tows the APC with an electromagnet,
detects a red LED "enemy" and reports it, and reports its position every 30 s.

Development follows "option C": logic is built and tested in ROS 2 on a laptop,
then ported to an ESP32. Full requirements: `docs/SPEC.md` (read it before
starting a new feature; do not import it here, it is too long for every session).
Its **Amendments** section (end of file) overrides the original text where they conflict.

## Commands

```bash
# macOS: everything ROS runs in Docker (ROS 2 Jazzy, Ubuntu 24.04), repo mounted at /ws
docker compose build                        # once, and after docker/Dockerfile changes
scripts/dev.sh "colcon build --symlink-install"
scripts/dev.sh "colcon test --packages-select d9_core && colcon test-result --verbose"
docker compose exec dev bash -l             # interactive shell (after any scripts/dev.sh call)

# d9_core without ROS (plain CMake + FetchContent GoogleTest; needs cmake on the host)
cmake -S src/d9_core -B build/host && cmake --build build/host && ctest --test-dir build/host --output-on-failure

# Inside the container, or natively on Ubuntu 24.04 / WSL2 with ROS 2 Jazzy:
source /opt/ros/jazzy/setup.bash

# Build everything (run from repo root = the workspace)
colcon build --symlink-install
source install/setup.bash

# Core library unit tests (fast, no ROS needed at runtime)
colcon test --packages-select d9_core && colcon test-result --verbose

# All tests
colcon test && colcon test-result --verbose

# Simulation: Gazebo world + D9 nodes
ros2 launch d9_sim sim.launch.py

# Fake teams (server, drone, HQ) in a second terminal
ros2 launch d9_fakes fakes.launch.py

# Send a test command by hand
ros2 topic pub --once /d9/cmd/clear d9_interfaces/msg/ClearCmd "{x: 100.0, y: 50.0}"

# Firmware (milestone M5+, PlatformIO)
pio run -d firmware
pio run -d firmware -t upload
```

## Repository layout

```
src/
  d9_core/        Plain C++ library: ALL decision logic. Unit tested with GoogleTest.
  d9_interfaces/  ROS 2 .msg definitions (ClearCmd, TowCmd, EnemyReport, ...)
  d9_nodes/       Thin ROS 2 wrappers: bridge, mission, navigation, enemy_detect, actuators
  d9_fakes/       Stand-ins for other teams: fake_server, fake_drone, fake_hq
  d9_sim/         Gazebo world, D9 + APC models, launch files
firmware/         PlatformIO ESP32 project; lib/d9_core is a symlink to src/d9_core
docs/SPEC.md      Software specification (source of truth for requirements); docs/spec.pdf original
docker/, compose.yaml, scripts/dev.sh   ROS 2 Jazzy dev container
```

## Architecture rules

IMPORTANT: `d9_core` must never include ROS or Arduino headers.
It is compiled unchanged into both the ROS nodes and the ESP32 firmware.
If core code needs time, I/O or logging, it receives values as function
arguments or goes through an interface defined in `d9_core/include/d9_core/ports.hpp`.

- Nodes in `d9_nodes` only: read topics → call core → publish results. No decision logic in nodes.
- Every core module exposes `void update(uint32_t now_ms, ...)`, called every loop cycle.
- No blocking anywhere: no `sleep`, no `delay()`, no busy-wait loops. Time comes in as `now_ms`.
- One task (clear or tow) runs at a time. A new command while busy → ack with `busy`, ignore it.
- Enemy detection is NOT a task. It runs every cycle in parallel with the active task.
- Each task state has a timeout. On timeout: stop motors, end task, publish failed `done`.
- Navigation on the road (FR-1, amendment A1): steer by the white road line (`LineFollower`),
  decide arrival from drone fixes (`ArrivalMonitor`); `GoToPoint` combines them. The road is a
  closed loop with sudden curves, so overshooting a target costs a lap. Free-space
  pivot-then-drive is only for short off-line moves (push, reverse, APC alignment) — M3.

## Core library constraints (ESP32-compatible)

- C++17. No exceptions, no RTTI, no `<iostream>`.
- No heap allocation after startup: no `new`, no growing `std::vector`/`std::string` in `update()`.
  Use `std::array`, fixed-size buffers, and plain structs.
- Use `float`, not `double` (ESP32 has a hardware FPU for float only).
- Pure functions where possible (e.g. `headingFromPoints`, `angleError`), so they are easy to test.
- Angles: always normalize to (-180, 180] with `normalizeAngle()` before comparing.

## Units and coordinate conventions

- Internal units: centimetres, degrees, milliseconds.
- 0° points along +x, angles increase counter-clockwise.
- The real drone coordinate system is UNKNOWN. All conversion lives in `d9_core/coords.hpp`.
  Never convert units or frames anywhere else.
- Drone provides x, y only (no heading). Heading = gyro, corrected from drone positions
  while driving straight. Never use a magnetometer (magnet + motors corrupt it).
- Line reading: `offset` in [-1, 1], positive = line LEFT of centre (so positive = turn CCW,
  like angles), plus a `detected` flag. Core never sees raw line-sensor values.
- Drive command: `DriveCommand{left, right}` as fractions of full track speed, positive = forward.
- Messages in `d9_interfaces` carry the server/drone frame; `d9_core` works in the internal frame.

## Topics

| Topic                 | Dir | Notes                                     |
|-----------------------|-----|-------------------------------------------|
| `/d9/pose`            | in  | x, y from drone (via server), noisy       |
| `/d9/line`            | in  | internal: line sensor → navigation        |
| `/d9/cmd/clear`       | in  | target x, y                               |
| `/d9/cmd/tow`         | in  | APC x, y + safe spot x, y                 |
| `/d9/cmd/stop`        | in  | always wins, stops motors immediately     |
| `/d9/status/position` | out | every 30 s                                |
| `/d9/status/enemy`    | out | IMMEDIATELY on detection, never batched   |
| `/d9/status/done`     | out | task name + success flag                  |
| `/d9/status/ack`      | out | every command received                    |

The server protocol is not decided yet. Only `bridge` knows about it;
the proposal is MQTT + JSON. Do not add server-specific code outside `bridge`.

## Time-critical path

The tank must move within 3 s of enemy identification, and the clock starts
at our `enemy` message. Target: < 500 ms from camera frame to publish.
- Never put anything slow (logging to file, waiting for acks) before the enemy publish.
- Any change to `enemy_detect` or `bridge` must keep the latency test in `d9_fakes/fake_hq` passing.

## Safety rules

- `stop` must stop motors within one loop cycle, from any state.
- Stop motors if no pose or command arrives within the watchdog timeout (`config.hpp`).
- On stop during a tow, keep the magnet state as it is (do not drop the APC mid-move).
- Motor commands are clamped to limits defined in `config.hpp`. Never bypass the clamp.

## Code style

- C++: `snake_case` files, `PascalCase` types, `camelCase` functions, `kConstant` constants,
  `member_` suffix for private members. Headers in `include/<package>/`.
- Python allowed only in `d9_fakes` and launch files (rclpy). Core and nodes are C++.
- Tunable numbers (tolerances, speeds, timeouts, push distance) live in `config.hpp`
  or ROS parameters, never as magic numbers in logic.
- Keep functions under ~50 lines. Comment the *why*, not the *what*.
- Format with `clang-format` (config in repo root) before committing.

## Testing rules

- Every new function in `d9_core` gets a GoogleTest test in `d9_core/test/`.
- State machines: test every transition, including timeouts and `stop` from each state.
- A bug found on hardware or in sim gets a failing test first, then the fix.
- Run `colcon test --packages-select d9_core` after any core change and before saying a task is done.
- `fake_drone` publishes noisy positions without heading on purpose. Do not "fix" that;
  it is how we test the heading estimation.

## Known unknowns (do not hardcode guesses)

| Unknown                         | Isolated in           | Waiting on        |
|---------------------------------|-----------------------|-------------------|
| Drone coordinate system, origin | `coords.hpp`          | Drone team        |
| Drone update rate and accuracy  | `config.hpp`          | Drone team        |
| Server protocol                 | `bridge` node         | Server team       |
| Magnet mechanism                | `actuators` (on/off)  | Mechanical team   |
| LED sensor on the real robot    | `enemy_detect` input  | Our team          |
| Line sensor on the real robot   | `/d9/line` input node | Our team          |
| Road: direction, line width, where targets sit | `config.hpp`, `GoToPoint` | Test-field owner |
| Fields for steps 3 and 4        | not implemented yet   | Tank, drone teams |
| Bucket / berm building (FR-7)   | not implemented yet   | Mechanical team   |

When a task touches one of these, ask before assuming a value.

## Workflow

- Work in small steps: one capability or one state at a time, tested before moving on.
- Before writing code for a new feature, check `docs/SPEC.md` for the requirement ID (FR-x)
  and mention it in the commit message, e.g. `FR-3: add final alignment state`.
- Do not modify `d9_interfaces` messages without asking: other nodes and fakes depend on them.
- Do not touch `firmware/` until milestone M5 unless asked.
- Commits: small, imperative messages. Never commit `build/`, `install/`, `log/`.
- Always version control: one branch per milestone (`m1-workspace`, `m2-...`), a commit after
  every step, push after every commit, PR to `main` when the milestone's tests pass.
  GitHub (`jon1606/d9software`, public) is reached through the GitHub MCP server.

## Current milestone

M1 — Workspace: done. All 5 packages build; `d9_core` has angles, geometry, the motor clamp,
`LineFollower`, `ArrivalMonitor` and `GoToPoint`, all unit tested.
Next: M2 — line-follow to a typed-in target in Gazebo: loop-road world with a white line,
sim line sensor, `fake_drone` noisy x, y, `navigation` + `actuators` nodes, `coords.hpp`,
heading fusion, pose watchdog.
