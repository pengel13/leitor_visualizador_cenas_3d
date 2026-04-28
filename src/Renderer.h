#pragma once

// ============================================================
// Renderer.h — Central draw system.
//
// The Renderer owns:
//   • The Phong shader (solid lit rendering)
//   • The Unlit shader (wireframe overlay + grid + axes)
//   • GPU buffers for the infinite ground grid
//   • GPU buffers for the XYZ coordinate axes
//
// Each call to render() does:
//   1. Clear color + depth buffers
//   2. For each Object3D in the scene:
//        a. Draw with Phong shader (GL_FILL)
//        b. If wireframeEnabled: redraw with Unlit shader (GL_LINE)
//   3. Draw ground grid (if showGrid)
//   4. Draw coordinate axes (if showAxes)
//
// The Renderer does NOT own the Scene or Camera — they are
// passed by const reference each frame.
// ============================================================

#include <memory>
#include <glad/glad.h>
#include "Scene.h"
#include "Camera.h"
#include "Shader.h"

class Renderer {
public:
    // ---- Rendering toggles ----
    bool wireframeEnabled = false; // draw additional wireframe pass
    bool showGrid         = true;  // draw XZ ground grid
    bool showAxes         = true;  // draw XYZ axis lines

    glm::vec3 clearColor = glm::vec3(0.12f, 0.12f, 0.15f); // dark background

    Renderer();
    ~Renderer();

    // Non-copyable (owns GPU resources)
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Build all GPU-owned resources (shaders, grid VAO, axes VAO).
    // Must be called after a valid OpenGL context exists.
    void init();

    // Render one complete frame.
    // Draws all objects in 'scene' as seen through 'camera'.
    void render(const Scene& scene, const Camera& camera);

private:
    // ---- Shader programs ----
    std::unique_ptr<Shader> phongShader; // Phong lit rendering
    std::unique_ptr<Shader> unlitShader; // solid color, no lighting

    // ---- Ground grid ----
    GLuint gridVAO       = 0;
    GLuint gridVBO       = 0;
    int    gridVertCount = 0;  // number of vertices (each line = 2 verts)

    // ---- Coordinate axes ----
    GLuint axesVAO       = 0;
    GLuint axesVBO       = 0;
    int    axesVertCount = 0;

    // Build grid geometry: lines in the XZ plane from -extent to +extent
    void buildGrid(int halfExtent, int step);

    // Build axes geometry: three coloured lines along X/Y/Z
    void buildAxes(float length);

    // Draw helpers
    void drawGrid(const Camera& camera) const;
    void drawAxes(const Camera& camera) const;

    // Upload Phong-specific per-object uniforms
    void applyMaterial(Shader& sh, const Material&   mat)   const;
    void applyLight   (Shader& sh, const PointLight& light) const;
};
