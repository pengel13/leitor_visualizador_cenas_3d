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
    , material(outro.material)
    , textureID(outro.textureID)
    , VAO(outro.VAO), VBO(outro.VBO), EBO(outro.EBO)
    , contadorIndices(outro.contadorIndices)
{
    outro.VAO = outro.VBO = outro.EBO = 0;
    outro.contadorIndices = 0;
    outro.textureID = 0;
}

Malha& Malha::operator=(Malha&& outro) noexcept {
    if (this != &outro) {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);

        vertices        = std::move(outro.vertices);
        indices         = std::move(outro.indices);
        material        = outro.material;
        textureID       = outro.textureID;
        VAO             = outro.VAO; VBO = outro.VBO; EBO = outro.EBO;
        contadorIndices = outro.contadorIndices;

        outro.VAO = outro.VBO = outro.EBO = 0;
        outro.contadorIndices = 0;
        outro.textureID = 0;
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

    // location 0: posicao (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertice),
                          reinterpret_cast<void*>(offsetof(Vertice, posicao)));

    // location 1: normal (vec3)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertice),
                          reinterpret_cast<void*>(offsetof(Vertice, normal)));

    // location 2: coordenadas de textura (vec2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertice),
                          reinterpret_cast<void*>(offsetof(Vertice, coordTex)));

    glBindVertexArray(0);
}

// Desenha somente a geometria — sem fazer upload de material ou textura.
// Usado pelo wireframe overlay, onde o shader nao espera esses uniforms.
void Malha::desenhar() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, contadorIndices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// Faz upload do material e da textura para o shader antes de desenhar.
// Se a malha possui uma textura difusa (textureID != 0), ela e vinculada
// na unidade 0 e o shader e informado com temTextura=true.
// Caso contrario, o shader usa a cor do material como albedo.
void Malha::desenhar(Shader& shader) const {
    // Upload das propriedades do material Phong
    shader.definirVec3 ("material_ambient",   material.ambiente);
    shader.definirVec3 ("material_diffuse",   material.difuso);
    shader.definirVec3 ("material_specular",  material.especular);
    shader.definirFloat("material_shininess", material.brilho);
    shader.definirVec3 ("material_color",     material.cor);

    // Vincula a textura difusa (se existir) e informa o shader
    if (textureID != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        shader.definirInt ("textureDifusa", 0);
        shader.definirBool("temTextura",    true);
    } else {
        shader.definirBool("temTextura", false);
    }

    // Desenha a geometria
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, contadorIndices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    // Restaura estado de textura para evitar contaminacao entre malhas
    if (textureID != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
