#pragma once

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "Model.h"
#include "Shader.h"

struct Material {
    glm::vec3 ambiente  = glm::vec3(0.1f);
    glm::vec3 difuso    = glm::vec3(0.8f, 0.2f, 0.2f);
    glm::vec3 especular = glm::vec3(0.5f);
    float     brilho    = 32.f;
    glm::vec3 cor       = glm::vec3(0.8f, 0.8f, 0.8f);
};

class Objeto3D {
public:
    std::string             nome;
    std::shared_ptr<Modelo> modelo;
    Material                material;

    glm::vec3 posicao = glm::vec3(0.f);
    glm::vec3 rotacao = glm::vec3(0.f);
    glm::vec3 escala  = glm::vec3(1.f);

    bool estaSelecionado = false;

    Objeto3D(const std::string& nome, std::shared_ptr<Modelo> modelo);

    glm::mat4 obterMatrizModelo() const;
    void desenhar(Shader& shader) const;

    void transladar(const glm::vec3& delta);
    void rotacionar(const glm::vec3& deltaDeg);
    void escalarPor(const glm::vec3& fatores);
    void escalarPor(float fatorUniforme);
    void resetarTransformacao();
};
