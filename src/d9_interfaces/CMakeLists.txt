cmake_minimum_required(VERSION 3.16)
project(d9_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/DronePosition.msg"
  "msg/LineReading.msg"
  "msg/ClearCmd.msg"
  "msg/TowCmd.msg"
  "msg/PositionReport.msg"
  "msg/EnemyReport.msg"
  "msg/TaskDone.msg"
  "msg/CommandAck.msg"
  DEPENDENCIES builtin_interfaces
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()
