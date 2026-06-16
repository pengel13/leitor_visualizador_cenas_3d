#include "Curva.h"

// Formula de Bernstein para Bezier cubica: B(t) = u^3*P0 + 3u^2t*P1 + 3ut^2*P2 + t^3*P3
glm::vec3 avaliarBezier(const CurvaBezier& curva, float t) {
    float u = 1.f - t;
    return (u * u * u)           * curva.pontos[0]
         + (3.f * u * u * t)     * curva.pontos[1]
         + (3.f * u * t * t)     * curva.pontos[2]
         + (t * t * t)           * curva.pontos[3];
}
