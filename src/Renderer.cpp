#include "Renderer.h"

#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ============================================================
// Constructor / Destructor
// Note: init() must be called AFTER a valid OpenGL context exists.
// ============================================================
Renderer::Renderer()  = default;

Renderer::~Renderer() {
    // Release grid GPU resources
    if (gridVAO) glDeleteVertexArrays(1, &gridVAO);
    if (gridVBO) glDeleteBuffers(1, &gridVBO);

    // Release axes GPU resources
    if (axesVAO) glDeleteVertexArrays(1, &axesVAO);
    if (axesVBO) glDeleteBuffers(1, &axesVBO);
}

// ============================================================
// init — Compile shaders and build grid/axes geometry.
// Must be called once after glfwMakeContextCurrent().
// ============================================================
void Renderer::init() {
    // Compile the Phong lit shader (vertex + fragment)
    phongShader = std::make_unique<Shader>("shaders/phong.vert",
                                          "shaders/phong.frag");

    // Compile the unlit solid-color shader (wireframe, grid, axes)
    unlitShader = std::make_unique<Shader>("shaders/unlit.vert",
                                          "shaders/unlit.frag");

    // Build a 40×40 grid centred at origin in the XZ plane
    buildGrid(20, 1);

    // Build 40-unit long XYZ axes
    buildAxes(20.f);

    // Enable depth testing so closer fragments occlude further ones
    glEnable(GL_DEPTH_TEST);

    // Slightly smoother lines (driver may ignore on some GPUs)
    glEnable(GL_LINE_SMOOTH);

    std::cout << "[Renderer] Initialised.\n";
}

// ============================================================
// buildGrid — Generate line geometry for a ground grid.
//
// Lines run in the XZ plane (Y = 0).
//   halfExtent — how many units from origin to edge
//   step       — distance between lines
//
// Vertex layout: position (vec3) + color (vec3) = 6 floats
// Minor lines are grey; the X and Z centre-lines are slightly
// brighter so they stand out as cardinal axes.
// ============================================================
void Renderer::buildGrid(int halfExtent, int step) {
    // 6 floats per vertex: x,y,z, r,g,b
    std::vector<float> verts;
    verts.reserve(static_cast<size_t>(((halfExtent / step) * 4 + 4) * 6 * 2));

    auto addLine = [&](float x0, float y0, float z0,
                       float x1, float y1, float z1,
                       float r,  float g,  float b) {
        verts.insert(verts.end(), {x0,y0,z0, r,g,b, x1,y1,z1, r,g,b});
    };

    const float grey    = 0.3f;
    const float bright  = 0.5f;
    const float e       = static_cast<float>(halfExtent);

    // Lines parallel to X axis (run along X, separated in Z)
    for (int z = -halfExtent; z <= halfExtent; z += step) {
        float fz  = static_cast<float>(z);
        float col = (z == 0) ? bright : grey; // Z=0 line is X-axis on ground
        addLine(-e, 0.f, fz,  e, 0.f, fz,  col, col, col);
    }

    // Lines parallel to Z axis (run along Z, separated in X)
    for (int x = -halfExtent; x <= halfExtent; x += step) {
        float fx  = static_cast<float>(x);
        float col = (x == 0) ? bright : grey; // X=0 line is Z-axis on ground
        addLine(fx, 0.f, -e,  fx, 0.f, e,  col, col, col);
    }

    gridVertCount = static_cast<int>(verts.size() / 6);

    // Upload to GPU
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_STATIC_DRAW);

    // position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), reinterpret_cast<void*>(0));

    // color attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    glBindVertexArray(0);
}

// ============================================================
// buildAxes — Build three coloured lines for the world axes.
//
// X = red, Y = green, Z = blue (RGB = XYZ — a common convention)
// Each axis runs from -length to +length.
// ============================================================
void Renderer::buildAxes(float length) {
    // 6 floats per vertex (position + color), 2 verts × 3 axes
    std::vector<float> verts = {
        // X axis — red
        -length, 0.f, 0.f,    1.f, 0.2f, 0.2f,
         length, 0.f, 0.f,    1.f, 0.2f, 0.2f,
        // Y axis — green
        0.f, -length, 0.f,    0.2f, 1.f, 0.2f,
        0.f,  length, 0.f,    0.2f, 1.f, 0.2f,
        // Z axis — blue
        0.f, 0.f, -length,    0.2f, 0.2f, 1.f,
        0.f, 0.f,  length,    0.2f, 0.2f, 1.f,
    };

    axesVertCount = static_cast<int>(verts.size() / 6);

    glGenVertexArrays(1, &axesVAO);
    glGenBuffers(1, &axesVBO);

    glBindVertexArray(axesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    glBindVertexArray(0);
}

// ============================================================
// render — Draw one complete frame.
//
// Pass order:
//   1. Clear
//   2. Solid (Phong) pass for all objects
//   3. Wireframe pass for all objects (if enabled)
//   4. Grid
//   5. Axes
// ============================================================
void Renderer::render(const Scene& scene, const Camera& camera) {
    // ---- Clear ----
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Common matrices — computed once and shared by all draw calls
    glm::mat4 view       = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix();
    glm::vec3 viewPos    = camera.position;

    // ---- Solid / Phong pass ----
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    phongShader->use();

    // Upload matrices and camera position (shared across all objects)
    phongShader->setMat4("view",       view);
    phongShader->setMat4("projection", projection);
    phongShader->setVec3("viewPos",    viewPos);

    // Upload light uniforms (one point light for now)
    applyLight(*phongShader, scene.light);

    // Draw each object with its own model matrix and material
    for (const auto& obj : scene.objects) {
        applyMaterial(*phongShader, obj->material);
        obj->draw(*phongShader);
    }

    // ---- Wireframe overlay pass ----
    // Drawn on top of the solid pass using glPolygonOffset to avoid Z-fighting.
    if (wireframeEnabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // Offset wireframe slightly toward camera so it sits on top of the fill
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.f, -1.f);

        unlitShader->use();
        unlitShader->setMat4("view",           view);
        unlitShader->setMat4("projection",     projection);
        unlitShader->setVec3("color",          glm::vec3(0.95f, 0.95f, 0.95f));
        // Mesh VAO has normals at attribute 1, NOT vertex colours.
        // useVertexColor=false tells the fragment shader to ignore attribute 1
        // and use only the uniform colour.
        unlitShader->setBool("useVertexColor", false);

        for (const auto& obj : scene.objects) {
            glm::mat4 modelMat = obj->getModelMatrix();
            unlitShader->setMat4("model", modelMat);
            if (obj->model) obj->model->draw();
        }

        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // ---- Grid and Axes (always GL_LINE, no lighting) ----
    // Reset polygon mode to fill before drawing lines
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (showGrid) drawGrid(camera);
    if (showAxes) drawAxes(camera);
}

// ============================================================
// drawGrid — Render the XZ ground grid lines.
// Uses an identity model matrix (grid is in world space).
// ============================================================
void Renderer::drawGrid(const Camera& camera) const {
    unlitShader->use();
    unlitShader->setMat4("view",           camera.getViewMatrix());
    unlitShader->setMat4("projection",     camera.getProjectionMatrix());
    unlitShader->setMat4("model",          glm::mat4(1.f)); // no transform
    unlitShader->setVec3("color",          glm::vec3(1.f)); // white tint = passthrough
    // Grid VAO has real per-vertex colours at attribute 1 — use them
    unlitShader->setBool("useVertexColor", true);

    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, gridVertCount);
    glBindVertexArray(0);
}

// ============================================================
// drawAxes — Render the XYZ axis lines.
// ============================================================
void Renderer::drawAxes(const Camera& camera) const {
    unlitShader->use();
    unlitShader->setMat4("view",           camera.getViewMatrix());
    unlitShader->setMat4("projection",     camera.getProjectionMatrix());
    unlitShader->setMat4("model",          glm::mat4(1.f));
    unlitShader->setVec3("color",          glm::vec3(1.f)); // white tint = passthrough
    // Axes VAO has real per-vertex colours (R/G/B for X/Y/Z) — use them
    unlitShader->setBool("useVertexColor", true);

    glLineWidth(2.f); // thicker lines for axes

    glBindVertexArray(axesVAO);
    glDrawArrays(GL_LINES, 0, axesVertCount);
    glBindVertexArray(0);

    glLineWidth(1.f); // restore
}

// ============================================================
// applyMaterial — Upload Phong material uniforms for one object.
// Uniform names must match exactly what phong.frag declares.
// ============================================================
void Renderer::applyMaterial(Shader& sh, const Material& mat) const {
    sh.setVec3 ("material_ambient",   mat.ambient);
    sh.setVec3 ("material_diffuse",   mat.diffuse);
    sh.setVec3 ("material_specular",  mat.specular);
    sh.setFloat("material_shininess", mat.shininess);
    sh.setVec3 ("material_color",     mat.color);
}

// ============================================================
// applyLight — Upload point-light uniforms.
// ============================================================
void Renderer::applyLight(Shader& sh, const PointLight& light) const {
    sh.setVec3 ("light_position",  light.position);
    sh.setVec3 ("light_ambient",   light.ambient);
    sh.setVec3 ("light_diffuse",   light.diffuse);
    sh.setVec3 ("light_specular",  light.specular);
    sh.setFloat("light_constant",  light.constant);
    sh.setFloat("light_linear",    light.linear);
    sh.setFloat("light_quadratic", light.quadratic);
}
