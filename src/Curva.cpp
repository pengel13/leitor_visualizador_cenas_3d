#include "Curva.h"

// Avalia a curva de Bezier cubica usando a formula de Bernstein expandida.
// t deve estar no intervalo [0, 1].
// Fora desse intervalo a curva e extrapolada (comportamento esperado se
// o chamador nao limitar t corretamente).
glm::vec3 avaliarBezier(const CurvaBezier& curva, float t) {
    float u = 1.f - t;

    // Coeficientes binomiais de Bernstein para grau 3: 1, 3, 3, 1
    // B(t) = u^3*P0 + 3*u^2*t*P1 + 3*u*t^2*P2 + t^3*P3
    return (u * u * u)           * curva.pontos[0]
         + (3.f * u * u * t)     * curva.pontos[1]
         + (3.f * u * t * t)     * curva.pontos[2]
         + (t * t * t)           * curva.pontos[3];
}
