#include "Mesh.h"

#include <utility>
#include <iostream>

Malha::Malha(const std::vector<Vertice>& verts, const std::vector<unsigned int>& idxs)
    : vertices(verts)
    , indices(idxs)
    , contadorIndices(static_cast<GLsizei>(idxs.size()))
{
    configurarBuffersGPU();
}

Malha::~Malha() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

Malha::Malha(Malha&& outro) noexcept
    : vertices(std::move(outro.vertices))
    , indices (std::move(outro.indices))
    , VAO(outro.VAO), VBO(outro.VBO), EBO(outro.EBO)
    , contadorIndices(outro.contadorIndices)
{
    outro.VAO = outro.VBO = outro.EBO = 0;
    outro.contadorIndices = 0;
}

Malha& Malha::operator=(Malha&& outro) noexcept {
    if (this != &outro) {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);

        vertices        = std::move(outro.vertices);
        indices         = std::move(outro.indices);
        VAO             = outro.VAO; VBO = outro.VBO; EBO = outro.EBO;
        contadorIndices = outro.contadorIndices;

        outro.VAO = outro.VBO = outro.EBO = 0;
        outro.contadorIndices = 0;
    }
    return *this;
}

void Malha::configurarBuffersGPU() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertice)),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                 indices.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertice),
                          reinterpret_cast<void*>(offsetof(Vertice, posicao)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertice),
                          reinterpret_cast<void*>(offsetof(Vertice, normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertice),
                          reinterpret_cast<void*>(offsetof(Vertice, coordTex)));

    glBindVertexArray(0);
}

void Malha::desenhar() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, contadorIndices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
