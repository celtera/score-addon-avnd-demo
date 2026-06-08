cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

if(EMSCRIPTEN)
  return()
endif()

# Make Avendish's AvendishAddon.cmake findable, in every build context:
#  - score in-tree build: ScoreAvndHelper already put avendish/cmake on the module path
#  - standalone vs score SDK/source: avendish ships inside score
#  - pure Avendish: fetch it (only the cmake modules + headers are needed here)
if(DEFINED SCORE_SOURCE_DIR)
  list(APPEND CMAKE_MODULE_PATH "${SCORE_SOURCE_DIR}/3rdparty/avendish/cmake")
elseif(NOT TARGET score_lib_base AND NOT DEFINED SCORE_SDK)
  include(FetchContent)
  FetchContent_Declare(
    avendish
    GIT_REPOSITORY "https://github.com/celtera/avendish"
    GIT_TAG addon-cmake)
  FetchContent_Populate(avendish)
  list(APPEND CMAKE_MODULE_PATH "${avendish_SOURCE_DIR}/cmake")
endif()

# Detects the build context, bootstraps the score SDK if requested, and defines
# avnd_addon_init / avnd_addon_object / avnd_addon_finalize. Include before project().
include(AvendishAddon)

project(score_addon_avnd_demo CXX)

avnd_addon_init(NAME score_addon_avnd_demo)

avnd_addon_object(
  BASE score_addon_avnd_demo
  C_NAME avnd_addon_demo
  CLASS DemoProcessor
  SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Demo.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Demo.cpp")

avnd_addon_finalize(
  NAME score_addon_avnd_demo
  UUID 0b6a7c1e-9d24-4e3a-8f15-2c7b4a90d6e1
  VERSION 1.0.0)
