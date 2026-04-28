#include "Primitives.h"

#include <cmath>
#include <vector>
#include <glm/glm.hpp>

// Evita dependencia de _USE_MATH_DEFINES no MSVC
static constexpr float PI = 3.14159265358979323846f;

Malha Primitivos::criarCubo(float t) {
    float h = t * 0.5f;

    std::vector<Vertice>      vertices;
    std::vector<unsigned int> indices;
    vertices.reserve(24);
    indices.reserve(36);

    auto adicionarFace = [&](
        glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
        glm::vec3 norm)
    {
        unsigned int base = static_cast<unsigned int>(vertices.size());

        glm::vec2 uvs[4] = {{0.f,0.f},{1.f,0.f},{1.f,1.f},{0.f,1.f}};
        glm::vec3 pts[4] = { p0, p1, p2, p3 };

        for (int i = 0; i < 4; ++i) {
            Vertice v;
            v.posicao  = pts[i];
            v.normal   = norm;
            v.coordTex = uvs[i];
            vertices.push_back(v);
        }

        indices.insert(indices.end(), {
            base+0, base+1, base+2,
            base+0, base+2, base+3
        });
    };

    adicionarFace({-h,-h, h}, { h,-h, h}, { h, h, h}, {-h, h, h}, { 0, 0, 1});
    adicionarFace({ h,-h,-h}, {-h,-h,-h}, {-h, h,-h}, { h, h,-h}, { 0, 0,-1});
    adicionarFace({-h,-h,-h}, {-h,-h, h}, {-h, h, h}, {-h, h,-h}, {-1, 0, 0});
    adicionarFace({ h,-h, h}, { h,-h,-h}, { h, h,-h}, { h, h, h}, { 1, 0, 0});
    adicionarFace({-h, h, h}, { h, h, h}, { h, h,-h}, {-h, h,-h}, { 0, 1, 0});
    adicionarFace({-h,-h,-h}, { h,-h,-h}, { h,-h, h}, {-h,-h, h}, { 0,-1, 0});

    return Malha(vertices, indices);
}

Malha Primitivos::criarEsfera(float raio, int setores, int pilhas) {
    if (setores < 3) setores = 3;
    if (pilhas  < 2) pilhas  = 2;

    std::vector<Vertice>      vertices;
    std::vector<unsigned int> indices;

    const float passoSetor = 2.f * PI / static_cast<float>(setores);
    const float passoPilha =       PI / static_cast<float>(pilhas);

    for (int i = 0; i <= pilhas; ++i) {
        float phi = PI / 2.f - static_cast<float>(i) * passoPilha;
        float xz  = raio * std::cos(phi);
        float y   = raio * std::sin(phi);

        for (int j = 0; j <= setores; ++j) {
            float theta = static_cast<float>(j) * passoSetor;

            Vertice v;
            v.posicao.x  = xz * std::cos(theta);
            v.posicao.y  = y;
            v.posicao.z  = xz * std::sin(theta);
            v.normal     = glm::normalize(v.posicao);
            v.coordTex.x = static_cast<float>(j) / static_cast<float>(setores);
            v.coordTex.y = static_cast<float>(i) / static_cast<float>(pilhas);

            vertices.push_back(v);
        }
    }

    for (int i = 0; i < pilhas; ++i) {
        int k1 = i * (setores + 1);
        int k2 = k1 + setores + 1;

        for (int j = 0; j < setores; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(static_cast<unsigned int>(k1));
                indices.push_back(static_cast<unsigned int>(k2));
                indices.push_back(static_cast<unsigned int>(k1 + 1));
            }
            if (i != pilhas - 1) {
                indices.push_back(static_cast<unsigned int>(k1 + 1));
                indices.push_back(static_cast<unsigned int>(k2));
                indices.push_back(static_cast<unsigned int>(k2 + 1));
            }
        }
    }

    return Malha(vertices, indices);
}

Malha Primitivos::criarPlano(float t) {
    std::vector<Vertice> vertices;
    vertices.reserve(4);

    auto v = [](float x, float z, float u, float w) -> Vertice {
        Vertice vtx;
        vtx.posicao  = {x, 0.f, z};
        vtx.normal   = {0.f, 1.f, 0.f};
        vtx.coordTex = {u, w};
        return vtx;
    };

    vertices.push_back(v(-t, -t, 0.f, 0.f));
    vertices.push_back(v( t, -t, 1.f, 0.f));
    vertices.push_back(v( t,  t, 1.f, 1.f));
    vertices.push_back(v(-t,  t, 0.f, 1.f));

    std::vector<unsigned int> indices = { 0, 1, 2,  0, 2, 3 };

    return Malha(vertices, indices);
}
