#pragma once

#include <glm/glm.hpp>

// Representa uma curva de Bezier cubica definida por 4 pontos de controle.
// A curva parametrica vai de P0 (t=0) a P3 (t=1), com P1 e P2 como tangentes.
// O objeto animado percorre a curva com a velocidade definida em 'velocidade'
// (unidades de t por segundo). Quando loop=true, o t e reiniciado ao chegar em 1.
struct CurvaBezier {
    glm::vec3 pontos[4];           // P0, P1, P2, P3 em coordenadas de mundo
    float     velocidade = 0.3f;   // velocidade de percurso (t/segundo)
    bool      loop       = true;   // reinicia ao chegar no fim da curva
};

// Avalia a posicao na curva de Bezier cubica para o parametro t em [0, 1].
// Formula: B(t) = (1-t)^3*P0 + 3(1-t)^2*t*P1 + 3(1-t)*t^2*P2 + t^3*P3
glm::vec3 avaliarBezier(const CurvaBezier& curva, float t);
