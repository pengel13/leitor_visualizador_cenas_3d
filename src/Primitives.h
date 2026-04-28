#pragma once

// ============================================================
// Primitives.h — Procedural geometry generators.
//
// Generates Mesh objects for basic shapes without needing
// external model files.  Useful for testing and demo scenes.
//
// All shapes are centred at the origin with Y-up orientation.
// Normals are generated analytically (not from face cross products).
// ============================================================

#include "Mesh.h"

namespace Primitives {

    // ---- Cube -----------------------------------------------
    // A unit cube (side length 1) with 24 unique vertices
    // (each face has its own set so normals are face-flat).
    // Returns a Mesh with 36 indices (6 faces × 2 triangles × 3).
    // -----------------------------------------------------------
    Mesh createCube(float size = 1.0f);

    // ---- UV Sphere ------------------------------------------
    // A sphere approximated by a latitude/longitude grid.
    //   sectors = longitudinal divisions (columns, min 3)
    //   stacks  = latitudinal divisions  (rows,    min 2)
    // Higher values = smoother sphere.
    // -----------------------------------------------------------
    Mesh createSphere(float radius = 0.5f, int sectors = 36, int stacks = 18);

    // ---- Flat plane (XZ) ------------------------------------
    // A single quad in the XZ plane, useful as a ground surface.
    //   size = half-extent (total width = 2 * size)
    // -----------------------------------------------------------
    Mesh createPlane(float size = 1.0f);

} // namespace Primitives
