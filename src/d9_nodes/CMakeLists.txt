cmake_minimum_required(VERSION 3.16)
project(d9_nodes)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(d9_core REQUIRED)
find_package(d9_interfaces REQUIRED)

# Nodes arrive in M2 (navigation, actuators) and M3+ (mission, bridge, enemy_detect).

ament_package()
