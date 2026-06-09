#pragma once
#include <halp/controls.hpp>
#include <halp/geometry.hpp>
#include <halp/meta.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <numbers>
#include <vector>

// A minimal vec3, used to fill the geometry buffers.
struct vec3
{
  float x, y, z;
};

// MyGeometry is a geometry / mesh "generator": it has no geometry input, and
// produces a single mesh on its output every time it is ticked.
//
// The very same object compiles to:
//   - a TouchDesigner SOP (Surface Operator)
//   - a TouchDesigner POP (Particle Operator)
//   - a Godot GDExtension node producing an ArrayMesh
//   - an ossia score process
//
// See https://github.com/celtera/avendish/blob/main/examples/Tutorial/CubeGenerator.hpp
// for the canonical example this template is derived from.
class DemoGeometry
{
public:
  halp_meta(name, "My Geometry")
  halp_meta(c_name, "demo_geometry")
  halp_meta(category, "Generator")
  halp_meta(author, "Avendish")
  halp_meta(description, "Generate a subdivided, displaced plane mesh")

  // CHANGE THIS !!
  // - On linux: uuidgen | xargs printf | xclip -selection clipboard
  //   will copy one on the clipboard
  // - uuidgen exists on Mac and Windows too
  halp_meta(uuid, "3c2b1a09-7e6d-4c5b-9a8f-1d2e3f405162")

  // Named so that the UI (UI.hpp) can reference the ports via halp::item<&ins::...>.
  struct ins
  {
    // Width / depth of the plane.
    halp::hslider_f32<"Size", halp::range{.min = 0.1f, .max = 10.f, .init = 1.f}> size;

    // Number of subdivisions per side: (N+1) x (N+1) vertices, 2*N*N triangles.
    halp::spinbox_i32<"Subdivisions", halp::range{.min = 1, .max = 256, .init = 16}>
        subdivisions;

    // Amplitude of the sinusoidal displacement applied along the Y axis.
    halp::hslider_f32<"Height", halp::range{.min = 0.f, .max = 2.f, .init = 0.3f}> height;

    // Vertex color, applied uniformly across the mesh.
    halp::color_chooser<"Color"> color;
  } inputs;

  struct
  {
    // The geometry output. Avendish recognizes the mesh by its inner
    // halp::*_geometry member, and exposes the transform + dirty flags so that
    // hosts can avoid re-uploading unchanged data.
    struct
    {
      halp_meta(name, "Geometry")
      halp::position_normals_color_index_geometry mesh;
      float transform[16]{};
      bool dirty_mesh = false;
      bool dirty_transform = false;
    } geometry;
  } outputs;

  // Recompute the mesh and push it to the geometry output.
  //
  // Defined inline below: Avendish back-ends (TouchDesigner, Godot, ...) compile
  // this header directly and never link the MyGeometry library, so the object's
  // logic must live in the header, not in a separate .cpp.
  void operator()();


private:
  // Persistent storage backing the geometry buffers. The geometry output only
  // stores pointers into these vectors, so they must outlive each operator()
  // call until the host has consumed the mesh (i.e. at least until the next tick).
  std::vector<vec3> positions;
  std::vector<vec3> normals;
  std::vector<std::array<float, 4>> colors;
  std::vector<uint32_t> indices;
};

inline void DemoGeometry::operator()()
{
  const float size = inputs.size;
  const int n = inputs.subdivisions; // cells per side
  const float h = inputs.height;
  const std::array<float, 4> col{
      inputs.color.value.r, inputs.color.value.g, inputs.color.value.b,
      inputs.color.value.a};

  const int side = n + 1;            // vertices per side
  const int vertex_count = side * side;

  positions.clear();
  normals.clear();
  colors.clear();
  indices.clear();
  positions.reserve(vertex_count);
  normals.reserve(vertex_count);
  colors.reserve(vertex_count);
  indices.reserve(n * n * 6);

  // Sinusoidal displacement Y(u, v), with u, v in [0, 1].
  constexpr float tau = 2.f * std::numbers::pi_v<float>;
  const auto Y = [=](float u, float v) {
    return h * std::sin(u * tau) * std::cos(v * tau);
  };

  // Generate the grid of vertices on the XZ plane, displaced along Y.
  for(int j = 0; j < side; ++j)
  {
    const float v = (side > 1) ? float(j) / float(n) : 0.f;
    for(int i = 0; i < side; ++i)
    {
      const float u = (side > 1) ? float(i) / float(n) : 0.f;

      const float x = (u - 0.5f) * size;
      const float z = (v - 0.5f) * size;
      const float y = Y(u, v);
      positions.push_back({x, y, z});

      // Analytic normal from the partial derivatives of the surface.
      // dP/du = (size, dY/du, 0), dP/dv = (0, dY/dv, size).
      // normal = normalize(cross(dP/dv, dP/du)) -> points towards +Y.
      const float dYdu = h * tau * std::cos(u * tau) * std::cos(v * tau);
      const float dYdv = -h * tau * std::sin(u * tau) * std::sin(v * tau);
      float nx = -dYdu * size;
      float ny = size * size;
      float nz = -dYdv * size;
      const float len = std::sqrt(nx * nx + ny * ny + nz * nz);
      if(len > 0.f)
      {
        nx /= len;
        ny /= len;
        nz /= len;
      }
      normals.push_back({nx, ny, nz});

      colors.push_back(col);
    }
  }

  // Two counter-clockwise triangles per grid cell (front face towards +Y).
  for(int j = 0; j < n; ++j)
  {
    for(int i = 0; i < n; ++i)
    {
      const uint32_t v00 = uint32_t(j * side + i);
      const uint32_t v10 = v00 + 1;
      const uint32_t v01 = uint32_t((j + 1) * side + i);
      const uint32_t v11 = v01 + 1;

      indices.insert(indices.end(), {v00, v11, v10});
      indices.insert(indices.end(), {v00, v01, v11});
    }
  }

  // Point the geometry buffers at our persistent storage.
  auto& mesh = outputs.geometry.mesh;
  mesh.buffers.position_buffer.elements = reinterpret_cast<float*>(positions.data());
  mesh.buffers.position_buffer.element_count = positions.size();
  mesh.buffers.position_buffer.dirty = true;
  mesh.buffers.normal_buffer.elements = reinterpret_cast<float*>(normals.data());
  mesh.buffers.normal_buffer.element_count = normals.size();
  mesh.buffers.normal_buffer.dirty = true;
  mesh.buffers.color_buffer.elements = reinterpret_cast<float*>(colors.data());
  mesh.buffers.color_buffer.element_count = colors.size();
  mesh.buffers.color_buffer.dirty = true;
  mesh.buffers.index_buffer.elements = indices.data();
  mesh.buffers.index_buffer.element_count = indices.size();
  mesh.buffers.index_buffer.dirty = true;

  mesh.vertices = vertex_count;

  // Identity transform.
  std::memset(outputs.geometry.transform, 0, sizeof(outputs.geometry.transform));
  outputs.geometry.transform[0] = 1.f;
  outputs.geometry.transform[5] = 1.f;
  outputs.geometry.transform[10] = 1.f;
  outputs.geometry.transform[15] = 1.f;

  outputs.geometry.dirty_mesh = true;
  outputs.geometry.dirty_transform = true;
}
