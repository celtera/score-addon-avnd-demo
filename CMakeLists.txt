cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(score_addon_avnd_demo CXX)

# Use the Avendish the host already provides (e.g. ossia score); otherwise fetch it.
find_package(Avendish QUIET)
if(NOT Avendish_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    Avendish
    GIT_REPOSITORY "https://github.com/celtera/avendish"
    GIT_TAG addon-cmake)
  FetchContent_Populate(Avendish)
  list(APPEND CMAKE_PREFIX_PATH "${avendish_SOURCE_DIR}")
  find_package(Avendish REQUIRED)
endif()

avnd_addon_init(NAME score_addon_avnd_demo)

# A message object, via the default preset (CATEGORY object): Max/Pd/Python/TD-CHOP/…
avnd_addon_object(
  BASE score_addon_avnd_demo
  C_NAME avnd_addon_demo
  CLASS DemoProcessor
  SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Demo.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Demo.cpp")

# A geometry object, via explicit per-backend selection: a TouchDesigner POP (not a
# SOP), the Godot mesh node, the Max obj3d external and ossia — demonstrating BACKENDS.
avnd_addon_object(
  BASE score_addon_avnd_demo
  C_NAME avnd_addon_demo_geometry
  CLASS DemoGeometry
  BACKENDS ossia max:GEOMETRY touchdesigner:POP godot:GEOMETRY
  SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Geometry.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Geometry.cpp")

avnd_addon_finalize(
  NAME score_addon_avnd_demo
  UUID 0b6a7c1e-9d24-4e3a-8f15-2c7b4a90d6e1
  VERSION 1.0.0)
