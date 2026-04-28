#include "Renderer.h"

#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Renderizador::Renderizador()  = default;

Renderizador::~Renderizador() {
    if (gradeVAO) glDeleteVertexArrays(1, &gradeVAO);
    if (gradeVBO) glDeleteBuffers(1, &gradeVBO);
    if (eixosVAO) glDeleteVertexArrays(1, &eixosVAO);
    if (eixosVBO) glDeleteBuffers(1, &eixosVBO);
}

void Renderizador::inicializar() {
    shaderPhong   = std::make_unique<Shader>("shaders/phong.vert", "shaders/phong.frag");
    shaderSemLuz  = std::make_unique<Shader>("shaders/unlit.vert", "shaders/unlit.frag");

    construirGrade(20, 1);
    construirEixos(20.f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);

    std::cout << "[Renderizador] Inicializado.\n";
}

void Renderizador::construirGrade(int metadeExtensao, int passo) {
    std::vector<float> verts;
    verts.reserve(static_cast<size_t>(((metadeExtensao / passo) * 4 + 4) * 6 * 2));

    auto adicionarLinha = [&](float x0, float y0, float z0,
                               float x1, float y1, float z1,
                               float r,  float g,  float b) {
        verts.insert(verts.end(), {x0,y0,z0, r,g,b, x1,y1,z1, r,g,b});
    };

    const float cinza  = 0.3f;
    const float claro  = 0.5f;
    const float e      = static_cast<float>(metadeExtensao);

    for (int z = -metadeExtensao; z <= metadeExtensao; z += passo) {
        float fz  = static_cast<float>(z);
        float col = (z == 0) ? claro : cinza;
        adicionarLinha(-e, 0.f, fz,  e, 0.f, fz,  col, col, col);
    }

    for (int x = -metadeExtensao; x <= metadeExtensao; x += passo) {
        float fx  = static_cast<float>(x);
        float col = (x == 0) ? claro : cinza;
        adicionarLinha(fx, 0.f, -e,  fx, 0.f, e,  col, col, col);
    }

    contadorVertGrade = static_cast<int>(verts.size() / 6);

    glGenVertexArrays(1, &gradeVAO);
    glGenBuffers(1, &gradeVBO);

    glBindVertexArray(gradeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gradeVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    glBindVertexArray(0);
}

void Renderizador::construirEixos(float comprimento) {
    std::vector<float> verts = {
        -comprimento, 0.f, 0.f,    1.f, 0.2f, 0.2f,
         comprimento, 0.f, 0.f,    1.f, 0.2f, 0.2f,
        0.f, -comprimento, 0.f,    0.2f, 1.f, 0.2f,
        0.f,  comprimento, 0.f,    0.2f, 1.f, 0.2f,
        0.f, 0.f, -comprimento,    0.2f, 0.2f, 1.f,
        0.f, 0.f,  comprimento,    0.2f, 0.2f, 1.f,
    };

    contadorVertEixos = static_cast<int>(verts.size() / 6);

    glGenVertexArrays(1, &eixosVAO);
    glGenBuffers(1, &eixosVBO);

    glBindVertexArray(eixosVAO);
    glBindBuffer(GL_ARRAY_BUFFER, eixosVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    glBindVertexArray(0);
}

void Renderizador::renderizar(const Cena& cena, const Camera& camera) {
    glClearColor(corFundo.r, corFundo.g, corFundo.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 matVisao    = camera.obterMatrizVisao();
    glm::mat4 matProjecao = camera.obterMatrizProjecao();
    glm::vec3 posCamera   = camera.posicao;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    shaderPhong->usar();
    shaderPhong->definirMat4("view",       matVisao);
    shaderPhong->definirMat4("projection", matProjecao);
    shaderPhong->definirVec3("viewPos",    posCamera);
    aplicarLuz(*shaderPhong, cena.luz);

    for (const auto& obj : cena.objetos) {
        aplicarMaterial(*shaderPhong, obj->material);
        obj->desenhar(*shaderPhong);
    }

    if (wireframeAtivado) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.f, -1.f);

        shaderSemLuz->usar();
        shaderSemLuz->definirMat4("view",           matVisao);
        shaderSemLuz->definirMat4("projection",     matProjecao);
        shaderSemLuz->definirVec3("color",          glm::vec3(0.95f, 0.95f, 0.95f));
        // O VAO da Malha armazena normais no atributo 1, nao cores;
        // useVertexColor=false instrui o shader a usar a cor uniforme.
        shaderSemLuz->definirBool("useVertexColor", false);

        for (const auto& obj : cena.objetos) {
            shaderSemLuz->definirMat4("model", obj->obterMatrizModelo());
            if (obj->modelo) obj->modelo->desenhar();
        }

        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (mostrarGrade) desenharGrade(camera);
    if (mostrarEixos) desenharEixos(camera);
}

void Renderizador::desenharGrade(const Camera& camera) const {
    shaderSemLuz->usar();
    shaderSemLuz->definirMat4("view",           camera.obterMatrizVisao());
    shaderSemLuz->definirMat4("projection",     camera.obterMatrizProjecao());
    shaderSemLuz->definirMat4("model",          glm::mat4(1.f));
    shaderSemLuz->definirVec3("color",          glm::vec3(1.f));
    shaderSemLuz->definirBool("useVertexColor", true);

    glBindVertexArray(gradeVAO);
    glDrawArrays(GL_LINES, 0, contadorVertGrade);
    glBindVertexArray(0);
}

void Renderizador::desenharEixos(const Camera& camera) const {
    shaderSemLuz->usar();
    shaderSemLuz->definirMat4("view",           camera.obterMatrizVisao());
    shaderSemLuz->definirMat4("projection",     camera.obterMatrizProjecao());
    shaderSemLuz->definirMat4("model",          glm::mat4(1.f));
    shaderSemLuz->definirVec3("color",          glm::vec3(1.f));
    shaderSemLuz->definirBool("useVertexColor", true);

    glLineWidth(2.f);
    glBindVertexArray(eixosVAO);
    glDrawArrays(GL_LINES, 0, contadorVertEixos);
    glBindVertexArray(0);
    glLineWidth(1.f);
}

void Renderizador::aplicarMaterial(Shader& sh, const Material& mat) const {
    sh.definirVec3 ("material_ambient",   mat.ambiente);
    sh.definirVec3 ("material_diffuse",   mat.difuso);
    sh.definirVec3 ("material_specular",  mat.especular);
    sh.definirFloat("material_shininess", mat.brilho);
    sh.definirVec3 ("material_color",     mat.cor);
}

void Renderizador::aplicarLuz(Shader& sh, const LuzPontual& luz) const {
    sh.definirVec3 ("light_position",  luz.posicao);
    sh.definirVec3 ("light_ambient",   luz.ambiente);
    sh.definirVec3 ("light_diffuse",   luz.difuso);
    sh.definirVec3 ("light_specular",  luz.especular);
    sh.definirFloat("light_constant",  luz.constante);
    sh.definirFloat("light_linear",    luz.linear);
    sh.definirFloat("light_quadratic", luz.quadratico);
}
