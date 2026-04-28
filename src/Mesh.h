#pragma once

// ============================================================
// Mesh.h — GPU geometry buffer (VAO + VBO + EBO).
//
// A Mesh owns a block of vertex data on the GPU and knows how
// to draw it with a single glDrawElements call.
//
// Vertex layout (matches shader attribute locations):
//   location 0 → position   (vec3, 12 bytes)
//   location 1 → normal     (vec3, 12 bytes)
//   location 2 → texCoords  (vec2,  8 bytes)
//   Total stride: 32 bytes
//
// Data flow:
//   CPU vectors → glBufferData → GPU VRAM
//   On draw:  glBindVertexArray(VAO) + glDrawElements(...)
// ============================================================

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

// ---- Vertex -----------------------------------------------
// Per-vertex attributes stored interleaved in a single VBO.
// The layout MUST match the `layout(location = N)` declarations
// in every vertex shader that uses this mesh.
// -----------------------------------------------------------
struct Vertex {
    glm::vec3 position;   // attribute 0: object-space XYZ
    glm::vec3 normal;     // attribute 1: object-space normal direction
    glm::vec2 texCoords;  // attribute 2: UV texture coordinates
};

// ---- Mesh -------------------------------------------------
class Mesh {
public:
    // CPU-side copies (useful for debugging; not needed at draw time)
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;

    // OpenGL object handles
    GLuint VAO = 0;  // Vertex Array Object  — records attribute setup
    GLuint VBO = 0;  // Vertex Buffer Object — stores interleaved vertex data
    GLuint EBO = 0;  // Element Buffer Object — stores index list

    GLsizei indexCount = 0;  // number of indices to pass to glDrawElements

    // Upload vertex and index data to GPU, configure attribute pointers.
    // After construction the CPU vectors can be cleared if not needed.
    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<unsigned int>& indices);

    ~Mesh();

    // Non-copyable (owns GPU resources)
    Mesh(const Mesh&)            = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Move is allowed
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    // Issue the indexed draw call.
    // The caller is responsible for binding the correct shader and
    // setting all uniforms before calling this method.
    void draw() const;

private:
    // Allocate GPU buffers and configure glVertexAttribPointer
    void setupGPUBuffers();
};
