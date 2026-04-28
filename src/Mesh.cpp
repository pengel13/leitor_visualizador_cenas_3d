#include "Mesh.h"

#include <utility>  // std::move, std::swap
#include <iostream>

// ============================================================
// Constructor — Upload vertex and index data to GPU.
//
// Data flow:
//   CPU  std::vector<Vertex>  →  VBO (GL_ARRAY_BUFFER)
//   CPU  std::vector<uint>    →  EBO (GL_ELEMENT_ARRAY_BUFFER)
//
// The VAO records all attribute pointer setup so that at draw
// time we only need to bind the VAO and call glDrawElements.
// ============================================================
Mesh::Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& idxs)
    : vertices(verts)
    , indices(idxs)
    , indexCount(static_cast<GLsizei>(idxs.size()))
{
    setupGPUBuffers();
}

Mesh::~Mesh() {
    // Delete GPU objects if they were created
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

// ---- Move constructor / assignment -------------------------
// Transfers GPU handles without duplicating GL objects.
// ----------------------------------------------------------
Mesh::Mesh(Mesh&& other) noexcept
    : vertices(std::move(other.vertices))
    , indices (std::move(other.indices))
    , VAO(other.VAO), VBO(other.VBO), EBO(other.EBO)
    , indexCount(other.indexCount)
{
    // Null out the moved-from object so its destructor does nothing
    other.VAO = other.VBO = other.EBO = 0;
    other.indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        // Release own GPU resources first
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);

        vertices   = std::move(other.vertices);
        indices    = std::move(other.indices);
        VAO        = other.VAO;  VBO = other.VBO;  EBO = other.EBO;
        indexCount = other.indexCount;

        other.VAO = other.VBO = other.EBO = 0;
        other.indexCount = 0;
    }
    return *this;
}

// ============================================================
// setupGPUBuffers — Allocate VAO, VBO, EBO and configure
// vertex attribute pointers to match the shader layout:
//
//   layout(location = 0) in vec3 aPos;       // Vertex.position
//   layout(location = 1) in vec3 aNormal;    // Vertex.normal
//   layout(location = 2) in vec2 aTexCoords; // Vertex.texCoords
//
// The Vertex struct is:
//   struct Vertex { vec3 position; vec3 normal; vec2 texCoords; }
//   offset 0  → position  (3 floats = 12 bytes)
//   offset 12 → normal    (3 floats = 12 bytes)
//   offset 24 → texCoords (2 floats =  8 bytes)
//   stride = 32 bytes
// ============================================================
void Mesh::setupGPUBuffers() {
    // 1. Generate OpenGL object handles
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // 2. Bind VAO first so all subsequent buffer/attribute calls
    //    are recorded in it.
    glBindVertexArray(VAO);

    // 3. Upload vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(),
                 GL_STATIC_DRAW);  // data uploaded once, read many times

    // 4. Upload index data to EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                 indices.data(),
                 GL_STATIC_DRAW);

    // 5. Tell the VAO how to interpret the VBO data.
    //    Arguments: (attrib location, components, type, normalise, stride, offset)

    // Attribute 0: position (3 floats at byte offset 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position)));

    // Attribute 1: normal (3 floats at byte offset 12)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));

    // Attribute 2: texCoords (2 floats at byte offset 24)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, texCoords)));

    // 6. Unbind VAO (not the EBO — the EBO binding is stored in the VAO)
    glBindVertexArray(0);
}

// ============================================================
// draw — Issue the indexed draw call.
//
// glDrawElements reads indices from the EBO (bound inside VAO)
// and uses them to fetch vertices from the VBO.
// GL_TRIANGLES means every 3 consecutive indices form a triangle.
// ============================================================
void Mesh::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
