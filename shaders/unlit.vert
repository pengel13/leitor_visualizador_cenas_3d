#version 460 core

// ============================================================
// unlit.vert — Vertex shader for unlit (no lighting) geometry.
//
// Used for:
//   • Wireframe overlay
//   • Ground grid lines
//   • XYZ coordinate axes
//
// Each vertex carries a position and a per-vertex colour.
// The colour is multiplied by the 'color' uniform in the
// fragment shader, allowing the caller to tint all lines at once.
// ============================================================

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 v_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    v_color = aColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
