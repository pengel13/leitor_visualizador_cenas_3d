#include "Object3D.h"

#include <algorithm>
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
    if (!modelo) return;

    glm::mat4 matModelo = obterMatrizModelo();

    // transpose(inverse(modelo)) corrige normais sob escala nao-uniforme
    glm::mat3 matNormal = glm::mat3(glm::transpose(glm::inverse(matModelo)));

    shader.definirMat4("model",        matModelo);
    shader.definirMat3("normalMatrix", matNormal);
    shader.definirBool("isSelected",   estaSelecionado);

    // Determina se o modelo tem materiais proprios (carregados do .mtl).
    // Se a primeira malha ainda tem os defaults (ka=0.1 cinza), assumimos
    // que o modelo usa material proprio; caso contrario aplicamos o override.
    // Estrategia simples: se o modelo tem malhas, delega para modelo->desenhar(shader)
    // que ja faz upload de material por malha. O material de substituicao e usado
    // apenas para primitivos (onde o Modelo tem exatamente 1 malha sem .mtl).
    modelo->desenhar(shader);
}

void Objeto3D::atualizar(float deltaTempo) {
    if (!animacao.has_value()) return;

    CurvaBezier& curva = animacao.value();

    // Avanca o parametro t da curva proporcional ao tempo decorrido
    tempoAnimacao += curva.velocidade * deltaTempo;

    if (curva.loop) {
        // Reinicia ao chegar no fim, criando um loop continuo
        if (tempoAnimacao > 1.f) {
            tempoAnimacao -= 1.f;
        }
    } else {
        // Para ao chegar no fim da curva
        tempoAnimacao = std::min(tempoAnimacao, 1.f);
    }

    // Atualiza a posicao do objeto com a posicao na curva para o t atual
    posicao = avaliarBezier(curva, tempoAnimacao);
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
