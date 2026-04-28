#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

struct Vertice {
    glm::vec3 posicao;
    glm::vec3 normal;
    glm::vec2 coordTex;
};

class Malha {
public:
    std::vector<Vertice>      vertices;
    std::vector<unsigned int> indices;

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

    void desenhar() const;

private:
    void configurarBuffersGPU();
};
