#pragma once

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Object3D.h"

struct LuzPontual {
    glm::vec3 posicao  = glm::vec3(3.f, 5.f, 3.f);
    glm::vec3 ambiente = glm::vec3(0.15f);
    glm::vec3 difuso   = glm::vec3(1.0f);
    glm::vec3 especular = glm::vec3(1.0f);
    float constante  = 1.0f;
    float linear     = 0.045f;
    float quadratico = 0.0075f;
};

class Cena {
public:
    std::vector<std::unique_ptr<Objeto3D>> objetos;
    LuzPontual luz;
    int indiceAtivo = 0;

    void adicionarObjeto(std::unique_ptr<Objeto3D> obj);
    void selecionarProximo();
    void selecionarAnterior();

    Objeto3D*       obterAtivo();
    const Objeto3D* obterAtivo() const;

    int contarObjetos() const { return static_cast<int>(objetos.size()); }

    void imprimirStatus() const;

private:
    void sincronizarFlagsSelecao();
};
