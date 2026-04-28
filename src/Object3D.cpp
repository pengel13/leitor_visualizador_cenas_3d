#include "Object3D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Objeto3D::Objeto3D(const std::string& nomeObj, std::shared_ptr<Modelo> modeloObj)
    : nome(nomeObj)
    , modelo(std::move(modeloObj))
{}

glm::mat4 Objeto3D::obterMatrizModelo() const {
    glm::mat4 m = glm::mat4(1.f);
    m = glm::translate(m, posicao);
    m = glm::rotate(m, glm::radians(rotacao.x), glm::vec3(1.f, 0.f, 0.f));
    m = glm::rotate(m, glm::radians(rotacao.y), glm::vec3(0.f, 1.f, 0.f));
    m = glm::rotate(m, glm::radians(rotacao.z), glm::vec3(0.f, 0.f, 1.f));
    m = glm::scale(m, escala);
    return m;
}

void Objeto3D::desenhar(Shader& shader) const {
    glm::mat4 matModelo  = obterMatrizModelo();

    // transpose(inverse(modelo)) corrige normais sob escala nao-uniforme
    glm::mat3 matNormal = glm::mat3(glm::transpose(glm::inverse(matModelo)));

    shader.definirMat4("model",        matModelo);
    shader.definirMat3("normalMatrix", matNormal);
    shader.definirBool("isSelected",   estaSelecionado);

    if (modelo) {
        modelo->desenhar();
    }
}

void Objeto3D::transladar(const glm::vec3& delta)     { posicao += delta; }
void Objeto3D::rotacionar(const glm::vec3& deltaDeg)  { rotacao += deltaDeg; }
void Objeto3D::escalarPor(const glm::vec3& fatores)   { escala  *= fatores; }
void Objeto3D::escalarPor(float fatorUniforme)        { escala  *= fatorUniforme; }

void Objeto3D::resetarTransformacao() {
    posicao = glm::vec3(0.f);
    rotacao = glm::vec3(0.f);
    escala  = glm::vec3(1.f);
}
