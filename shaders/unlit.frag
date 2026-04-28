#version 460 core

// ============================================================
// unlit.frag — Fragment shader for unlit (no lighting) geometry.
//
// Two modes controlled by the 'useVertexColor' uniform:
//
//   useVertexColor = true  — output v_color * color
//     Used for: grid and axes, where each vertex carries a real colour.
//
//   useVertexColor = false — output color only (ignore v_color)
//     Used for: wireframe overlay, where attribute 1 is the mesh normal,
//     not a colour, and we want a single flat colour for all lines.
// ============================================================

in  vec3 v_color;
out vec4 fragColor;

// Uniform colour: tint when useVertexColor=true, full colour when false
uniform vec3 color;

// Switch between per-vertex colour and uniform-only colour
uniform bool useVertexColor;

void main() {
    if (useVertexColor) {
        // Grid / axes path: blend per-vertex colour with the uniform tint
        fragColor = vec4(v_color * color, 1.0);
    } else {
        // Wireframe path: ignore vertex data; use only the uniform colour
        fragColor = vec4(color, 1.0);
    }
}
