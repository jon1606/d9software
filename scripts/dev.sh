#!/usr/bin/env bash
# Run a command inside the ROS 2 Jazzy dev container, starting it if needed.
# Usage: scripts/dev.sh "colcon build --symlink-install"
set -euo pipefail
cd "$(dirname "$0")/.."
if [ -z "$(docker compose ps -q dev 2>/dev/null)" ]; then
  docker compose up -d dev >/dev/null
fi
exec docker compose exec -T dev bash -lc "$*"
