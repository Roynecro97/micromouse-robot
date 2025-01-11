if(CONFIG_MICROMOUSE_UNITTEST_MODE)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
  )

  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  get_property(GTEST_SRCS TARGET gtest PROPERTY SOURCES)
  get_property(GMOCK_SRCS TARGET gmock PROPERTY SOURCES)
  get_property(GTEST_SRC_DIR TARGET gtest PROPERTY SOURCE_DIR)
  get_property(GMOCK_SRC_DIR TARGET gmock PROPERTY SOURCE_DIR)
  list(TRANSFORM GTEST_SRCS PREPEND ${GTEST_SRC_DIR}/)
  list(TRANSFORM GMOCK_SRCS PREPEND ${GMOCK_SRC_DIR}/)

  set(GTEST_GMOCK_INCLUDES)
  foreach(src_dir IN ITEMS ${GTEST_SRC_DIR} ${GMOCK_SRC_DIR})
    foreach(include_subdir IN ITEMS "" include/)
      file(RELATIVE_PATH rel_include ${CMAKE_CURRENT_SOURCE_DIR} ${src_dir}/${include_subdir})
      list(APPEND GTEST_GMOCK_INCLUDES ${rel_include})
    endforeach()
  endforeach()

  idf_component_register(
    SRCS
      ${GTEST_SRCS}
      ${GMOCK_SRCS}

      unittests/physical_size_test.cc
      unittests/strongly_typed_test.cc
      unittests/typing_utils_test.cc
      unittests/value_range_test.cc

      unittests/cell_test.cc
      unittests/direction_test.cc
      unittests/maze_test.cc

      unittests/test_main.cc
    INCLUDE_DIRS
      ${GTEST_GMOCK_INCLUDES}
      unittests
  )
elseif(MICROMOUSE_MORSE_MODE)
  idf_component_register(
    SRCS
      main.cpp
    INCLUDE_DIRS
      .
  )
else()
  idf_component_register(
    SRCS
      distance_sensor.cpp
      kalman_filter.cpp
      main.cpp
      motion_model.cpp
      motor.cpp
      periodic_caller.cpp
      pid_controller.cpp
      segment.cpp
    INCLUDE_DIRS
      ../managed_components/espressif__eigen/eigen
  )
endif()
