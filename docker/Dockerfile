# ROS 2 Jazzy dev image for the Mini D9 workspace (Ubuntu 24.04, multi-arch).
FROM ros:jazzy

RUN apt-get update && apt-get install -y --no-install-recommends \
      ros-dev-tools \
      ros-jazzy-ament-cmake-gtest \
      clang-format \
    && rm -rf /var/lib/apt/lists/*

# Login shells (bash -l) get ROS and, once built, the workspace overlay.
RUN printf '%s\n' \
      'source /opt/ros/jazzy/setup.bash' \
      '[ -f /ws/install/setup.bash ] && source /ws/install/setup.bash' \
      > /etc/profile.d/ros.sh

WORKDIR /ws
