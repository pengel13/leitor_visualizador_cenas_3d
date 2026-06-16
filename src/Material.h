#pragma once

#include <glm/glm.hpp>

// Propriedades de material Phong para um objeto ou malha.
// Quando carregado de um arquivo .mtl via Assimp, os valores ka/kd/ks
// sao lidos de AI_MATKEY_COLOR_AMBIENT/DIFFUSE/SPECULAR.
// Para objetos primitivos (sem .mtl), os valores sao definidos manualmente.
struct Material {
    glm::vec3 ambiente  = glm::vec3(0.1f);           // ka — coeficiente de luz ambiente
    glm::vec3 difuso    = glm::vec3(0.8f, 0.2f, 0.2f); // kd — coeficiente difuso
    glm::vec3 especular = glm::vec3(0.5f);           // ks — coeficiente especular
    float     brilho    = 32.f;                       // expoente de Phong (shininess)
    glm::vec3 cor       = glm::vec3(0.8f, 0.8f, 0.8f); // cor base (albedo) — usada quando nao ha textura
};
