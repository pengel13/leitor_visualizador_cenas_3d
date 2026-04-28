#include "Camera.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Camera::Camera(glm::vec3 posInicial, float proporcao)
    : posicao(posInicial)
    , cimaMundo(0.f, 1.f, 0.f)
    , proporcaoTela(proporcao)
{
    atualizarVetores();
}

void Camera::atualizarVetores() {
    float guinadaRad  = glm::radians(guinada);
    float arfagemRad  = glm::radians(arfagem);

    frente.x = std::cos(guinadaRad) * std::cos(arfagemRad);
    frente.y = std::sin(arfagemRad);
    frente.z = std::sin(guinadaRad) * std::cos(arfagemRad);
    frente   = glm::normalize(frente);

    direita = glm::normalize(glm::cross(frente, cimaMundo));
    cima    = glm::normalize(glm::cross(direita, frente));
}

glm::mat4 Camera::obterMatrizVisao() const {
    return glm::lookAt(posicao, posicao + frente, cima);
}

glm::mat4 Camera::obterMatrizProjecao() const {
    if (modoProjecao == ModoProjecao::Perspectiva) {
        return glm::perspective(glm::radians(campoVisao), proporcaoTela, planoProximo, planoDistante);
    }

    float larg = escalaOrtografica * proporcaoTela;
    float alt  = escalaOrtografica;
    return glm::ortho(-larg, larg, -alt, alt, planoProximo, planoDistante);
}

void Camera::processarTeclado(Direcao dir, float deltaTempo) {
    float velocidade = velocidadeMovimento * deltaTempo;

    switch (dir) {
        case FRENTE:   posicao += frente    * velocidade; break;
        case TRAS:     posicao -= frente    * velocidade; break;
        case ESQUERDA: posicao -= direita   * velocidade; break;
        case DIREITA:  posicao += direita   * velocidade; break;
        case CIMA:     posicao += cimaMundo * velocidade; break;
        case BAIXO:    posicao -= cimaMundo * velocidade; break;
    }
}

void Camera::processarMovimentoMouse(float offsetX, float offsetY, bool limitarArfagem) {
    offsetX *= sensibilidadeMouse;
    offsetY *= sensibilidadeMouse;

    guinada  += offsetX;
    arfagem  += offsetY;

    if (limitarArfagem) {
        arfagem = std::clamp(arfagem, -89.f, 89.f);
    }

    atualizarVetores();
}

void Camera::processarRolaMouse(float offsetY) {
    campoVisao -= offsetY * 2.f;
    campoVisao  = std::clamp(campoVisao, 1.f, 90.f);
}

void Camera::alternarProjecao() {
    modoProjecao = (modoProjecao == ModoProjecao::Perspectiva)
                   ? ModoProjecao::Ortografica
                   : ModoProjecao::Perspectiva;
}
