#pragma once

#include <glm/glm.hpp>

enum class ModoProjecao { Perspectiva, Ortografica };

class Camera {
public:
    glm::vec3 posicao;
    glm::vec3 cimaMundo;

    float guinada  = -90.f;
    float arfagem  =   0.f;

    glm::vec3 frente;
    glm::vec3 direita;
    glm::vec3 cima;

    float velocidadeMovimento = 5.f;
    float sensibilidadeMouse  = 0.08f;

    float campoVisao        = 45.f;
    float planoProximo      = 0.1f;
    float planoDistante     = 500.f;
    float proporcaoTela     = 16.f / 9.f;
    float escalaOrtografica = 5.f;

    ModoProjecao modoProjecao = ModoProjecao::Perspectiva;

    explicit Camera(glm::vec3 posInicial = glm::vec3(0.f, 2.f, 8.f),
                    float     proporcao  = 16.f / 9.f);

    glm::mat4 obterMatrizVisao()    const;
    glm::mat4 obterMatrizProjecao() const;

    enum Direcao { FRENTE, TRAS, ESQUERDA, DIREITA, CIMA, BAIXO };
    void processarTeclado(Direcao dir, float deltaTempo);
    void processarMovimentoMouse(float offsetX, float offsetY, bool limitarArfagem = true);
    void processarRolaMouse(float offsetY);
    void alternarProjecao();
    void atualizarVetores();
};
