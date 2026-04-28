#version 460 core

// ============================================================
// phong.vert — Vertex shader for Phong illumination.
//
// This shader transforms each vertex from OBJECT SPACE to
// CLIP SPACE through three matrix multiplications:
//
//   Object space → World space  (model matrix)
//   World space  → Eye/Camera space  (view matrix)
//   Eye space    → Clip space   (projection matrix)
//
// It also passes the world-space position and normal to the
// fragment shader so that the Phong equation can be evaluated
// per-fragment (rather than per-vertex).
//
// Attribute layout (must match glVertexAttribPointer setup in Mesh.cpp):
//   location 0 → aPos       (vec3, object-space position)
//   location 1 → aNormal    (vec3, object-space normal)
//   location 2 → aTexCoords (vec2, UV texture coordinates)
// ============================================================

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// ---- Outputs to fragment shader ----
out vec3 v_fragPos;    // fragment position in world space
out vec3 v_normal;     // interpolated world-space normal
out vec2 v_texCoords;  // passthrough UV coordinates

// ---- Uniforms uploaded from CPU (Renderer.cpp) ----
uniform mat4 model;        // object → world space transform
uniform mat4 view;         // world  → camera space transform
uniform mat4 projection;   // camera → clip space transform

// Normal matrix = transpose(inverse(mat3(model))).
// Sent from the CPU because computing inverse on the GPU
// per-vertex is expensive and numerically imprecise.
// Required when model has non-uniform scale — otherwise normals
// would no longer be perpendicular to the surface.
uniform mat3 normalMatrix;

void main() {
    // Transform vertex position to world space
    // (used by the fragment shader for lighting equations)
    v_fragPos   = vec3(model * vec4(aPos, 1.0));

    // Transform normal to world space using the normal matrix.
    // normalize() re-unitizes after matrix multiplication may stretch.
    v_normal    = normalize(normalMatrix * aNormal);

    v_texCoords = aTexCoords;

    // Final clip-space position for rasterisation.
    // gl_Position is the only required output of a vertex shader.
    gl_Position = projection * view * vec4(v_fragPos, 1.0);
}
