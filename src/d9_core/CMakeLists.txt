cmake_minimum_required(VERSION 3.16)
project(d9_core LANGUAGES CXX)

# Builds two ways: as an ament package under colcon, or as plain CMake on a
# machine without ROS (fast unit-test loop). The library itself never sees ROS.

set(D9_CORE_SOURCES
  src/angles.cpp
  src/geometry.cpp
  src/drive.cpp
  src/line_follow.cpp
  src/arrival.cpp
  src/go_to_point.cpp
)
set(D9_CORE_TESTS
  test/test_angles.cpp
  test/test_geometry.cpp
  test/test_drive.cpp
  test/test_line_follow.cpp
  test/test_arrival.cpp
  test/test_go_to_point.cpp
)

add_library(d9_core STATIC ${D9_CORE_SOURCES})
target_include_directories(d9_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(d9_core PUBLIC cxx_std_17)
# Mirror the ESP32 constraints here so violations fail on the laptop, not at port time.
target_compile_options(d9_core PRIVATE
  -Wall -Wextra -Wpedantic -Wshadow -Wdouble-promotion -Werror
  -fno-exceptions -fno-rtti
)
set_target_properties(d9_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

find_package(ament_cmake QUIET)

if(ament_cmake_FOUND)
  install(DIRECTORY include/ DESTINATION include)
  install(TARGETS d9_core EXPORT export_d9_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )
  ament_export_targets(export_d9_core HAS_LIBRARY_TARGET)

  if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)
    ament_add_gtest(d9_core_test ${D9_CORE_TESTS})
    target_link_libraries(d9_core_test d9_core)
  endif()
else()
  include(CTest)
  if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
    )
    FetchContent_MakeAvailable(googletest)
    add_executable(d9_core_test ${D9_CORE_TESTS})
    target_link_libraries(d9_core_test d9_core GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(d9_core_test)
  endif()
endif()

if(BUILD_TESTING)
  add_test(NAME d9_core_purity
    COMMAND ${CMAKE_COMMAND} -DCORE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_forbidden.cmake
  )
endif()

if(ament_cmake_FOUND)
  ament_package()
endif()
