#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Material.h"
#include "Shader.h"

// Vertice armazena os tres atributos interpolados pelo rasterizador:
// posicao no espaco objeto, normal no espaco objeto e coordenadas de textura.
// O layout em memoria (posicao → normal → coordTex) e usado diretamente
// por glVertexAttribPointer via offsetof.
struct Vertice {
    glm::vec3 posicao;
    glm::vec3 normal;
    glm::vec2 coordTex;
};

// Malha representa uma unica submalha (grupo de faces) de um modelo 3D.
// Possui propriedades de material proprias (lidas do .mtl via Assimp)
// e opcionalmente um ID de textura difusa carregado do disco.
// A classe e move-only: ela fica dona dos handles OpenGL (VAO/VBO/EBO)
// e os libera no destrutor.
class Malha {
public:
    std::vector<Vertice>      vertices;
    std::vector<unsigned int> indices;

    Material material;          // ka/kd/ks/shininess lidos do .mtl (ou default)
    GLuint   textureID = 0;     // ID da textura difusa; 0 = sem textura

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    GLsizei contadorIndices = 0;

    Malha(const std::vector<Vertice>& vertices,
          const std::vector<unsigned int>& indices);

    ~Malha();

    Malha(const Malha&)            = delete;
    Malha& operator=(const Malha&) = delete;

    Malha(Malha&& outro) noexcept;
    Malha& operator=(Malha&& outro) noexcept;

    // Desenha a geometria pura (sem material) — usada pelo wireframe overlay
    void desenhar() const;

    // Faz upload do material e da textura para o shader, depois desenha a geometria.
    // Usada pelo passo de iluminacao Phong principal.
    void desenhar(Shader& shader) const;

private:
    // Cria e configura os buffers GPU (VAO, VBO, EBO) e os ponteiros de atributo.
    void configurarBuffersGPU();
};
