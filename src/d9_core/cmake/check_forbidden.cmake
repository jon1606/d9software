# Fails if d9_core code uses anything that would break the ESP32 port or pull in
# ROS/Arduino. Run by CTest: cmake -DCORE_DIR=<pkg dir> -P check_forbidden.cmake
# Only // comments are stripped before matching, so core code uses // comments.

file(GLOB_RECURSE core_files "${CORE_DIR}/include/*.hpp" "${CORE_DIR}/src/*.cpp")

set(forbidden
  "#include[ \t]*[<\"](rclcpp|rclpy|rcl|ros|Arduino)[^>\"]*[>\"]"
  "#include[ \t]*<(iostream|vector|string|map|list|thread)>"
  "[^A-Za-z0-9_]new[ \t]+[A-Za-z_]"
  "[^A-Za-z0-9_]double[^A-Za-z0-9_]"
  "[^A-Za-z0-9_](delay|sleep|sleep_for|malloc)[ \t]*\\("
)

set(violations "")
foreach(path IN LISTS core_files)
  file(READ "${path}" content)
  string(REGEX REPLACE "//[^\n]*" "" code "${content}")
  foreach(pattern IN LISTS forbidden)
    string(REGEX MATCH "${pattern}" hit " ${code}")
    if(hit)
      string(APPEND violations "\n  ${path}: '${hit}'")
    endif()
  endforeach()
endforeach()

if(violations)
  message(FATAL_ERROR "d9_core must stay ROS/Arduino-free and ESP32-safe:${violations}")
endif()
