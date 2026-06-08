# score-addon-avnd-demo

Validation addon for the **unified Avendish addon CMake** technique: a single
`CMakeLists.txt` that builds the same object both as

- an **ossia/score** add-on (only the ossia back-end), and
- a **standalone Avendish** object (Max/MSP, Pure Data, Python, TouchDesigner, Godot, …).

The rule: when the addon is built as `add_subdirectory` of ossia/score, only the ossia
back-end is produced; built on its own, all the standalone back-ends are.

## How it works

`avendish/cmake/AvendishAddon.cmake` is the single entry point. It detects the build
context and exposes three macros:

| Context | Detection | Back-ends |
|---|---|---|
| `add_subdirectory` of score | `score_lib_base` is a target | ossia only |
| standalone vs score SDK / source | `SCORE_SOURCE_DIR` / `SCORE_SDK` set → bootstraps it | ossia only |
| pure Avendish | none of the above → `find_package(Avendish)` | Max/Pd/Python/TD/Godot/… |

```cmake
include(AvendishAddon)              # before project(); bootstraps the score SDK if asked
project(score_addon_avnd_demo CXX)

avnd_addon_init(NAME score_addon_avnd_demo)
avnd_addon_object(
  BASE score_addon_avnd_demo
  C_NAME avnd_addon_demo
  CLASS DemoProcessor
  SOURCES src/Demo.hpp src/Demo.cpp)
avnd_addon_finalize(NAME score_addon_avnd_demo UUID <uuid> VERSION 1.0.0)
```

In a score build the macros forward to `avnd_score_plugin_init/add/finalize`; standalone
they forward to `add_library` + `avnd_make_object`. The object in `src/Demo.hpp` is
framework-agnostic (halp + `std` only) so it compiles in both.

## Building

Standalone (downloads Avendish, builds every back-end whose SDK is present):

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

As a score add-on: drop it in score's addon folder (or point `SCORE_SOURCE_DIR` /
`SCORE_SDK` at score) — only `score_addon_avnd_demo` is built.
