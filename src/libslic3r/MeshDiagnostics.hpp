#ifndef slic3r_MeshDiagnostics_hpp_
#define slic3r_MeshDiagnostics_hpp_

#include <admesh/stl.h>
#include <cstddef>

namespace Slic3r {

struct MeshDiagnosticStats {
    size_t non_manifold_edges    = 0;
    size_t non_manifold_vertices = 0;
    size_t open_edges            = 0;
};

// Detect topological defects on an indexed triangle set.
//
// Reported defects:
//   Open edge:           an undirected edge referenced by exactly 1 face.
//   Non-manifold edge:   an undirected edge shared by more than 2 faces.
//   Non-manifold vertex: a vertex whose incident faces do not form a single
//                        connected fan when traversed through shared edges.
MeshDiagnosticStats its_mesh_diagnostics(const indexed_triangle_set &its);

// Lightweight edge-only diagnostics. Counts open edges and non-manifold edges,
// but skips non-manifold vertex detection.
MeshDiagnosticStats its_edge_diagnostics(const indexed_triangle_set &its);

} // namespace Slic3r

#endif // slic3r_MeshDiagnostics_hpp_
