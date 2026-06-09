# score-addon-avnd-demo

Validation addon for the **unified Avendish addon CMake** technique: a single
`CMakeLists.txt` that builds the same object both as

- an **ossia/score** add-on (only the ossia back-end), and
- a **standalone Avendish** object (Max/MSP, Pure Data, Python, TouchDesigner, Godot, …).

The rule: when the addon is built as `add_subdirectory` of ossia/score, only the ossia
back-end is produced; built on its own, all the standalone back-ends are.

## How it works

The addon has **no knowledge of the host**. It just asks for Avendish:

```cmake
find_package(Avendish QUIET)        # use the host's Avendish (e.g. score) if present
if(NOT Avendish_FOUND)              # otherwise fetch it
  include(FetchContent)
  FetchContent_Declare(Avendish GIT_REPOSITORY https://github.com/celtera/avendish GIT_TAG …)
  FetchContent_Populate(Avendish)
  list(APPEND CMAKE_PREFIX_PATH "${avendish_SOURCE_DIR}")
  find_package(Avendish REQUIRED)
endif()

avnd_addon_init(NAME score_addon_avnd_demo)
avnd_addon_object(BASE score_addon_avnd_demo C_NAME avnd_addon_demo CLASS DemoProcessor
                  SOURCES src/Demo.hpp src/Demo.cpp)
avnd_addon_finalize(NAME score_addon_avnd_demo UUID <uuid> VERSION 1.0.0)
```

`find_package(Avendish)` (via `AvendishConfig.cmake`) is host-aware: when a host already
provides the `avnd_*` commands (score puts its in-tree copy on the prefix path) it reuses
them; otherwise it builds Avendish from the fetched sources. Either way it loads
`AvendishAddon.cmake`, which detects the context and exposes the three `avnd_addon_*`
macros:

| Context | Detection | Back-ends |
|---|---|---|
| as / against ossia score | `avnd_score_plugin_add` exists | ossia only, via `avnd_score_plugin_*` |
| pure Avendish | otherwise | Max/Pd/Python/TD/Godot/… via `avnd_make_object` |

The object in `src/Demo.hpp` is framework-agnostic (halp + `std` only) so it compiles in
both.

## Two objects, two ways of choosing back-ends

- `src/Demo.hpp` — a message object built with the default **preset** (`CATEGORY object`):
  Max / Pd / Python / TD-CHOP / Godot / ossia.
- `src/Geometry.hpp` — a mesh generator built with an **explicit back-end list**:
  `BACKENDS ossia max:GEOMETRY touchdesigner:POP godot:GEOMETRY` — i.e. a TouchDesigner
  **POP but not a SOP**, the Godot mesh node and the Max obj3d external. Each entry is
  `<backend>[:<PROCESSOR_TYPE>]` and maps straight to `avnd_make_<backend>`.

Use `CATEGORY` for a whole family at once, or `BACKENDS` for exact control. In a score
build both are ignored — score introspects the object and builds only its ossia process.

## Building

Standalone (downloads Avendish, builds every back-end whose SDK is present):

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

As a score add-on: drop it in score's addon folder (or point `SCORE_SOURCE_DIR` /
`SCORE_SDK` at score) — only `score_addon_avnd_demo` is built.
