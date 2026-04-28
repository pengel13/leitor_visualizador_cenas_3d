#pragma once

#include <memory>
#include <glad/glad.h>
#include "Scene.h"
#include "Camera.h"
#include "Shader.h"

class Renderizador {
public:
    bool wireframeAtivado = false;
    bool mostrarGrade     = true;
    bool mostrarEixos     = true;

    glm::vec3 corFundo = glm::vec3(0.12f, 0.12f, 0.15f);

    Renderizador();
    ~Renderizador();

    Renderizador(const Renderizador&)            = delete;
    Renderizador& operator=(const Renderizador&) = delete;

    void inicializar();
    void renderizar(const Cena& cena, const Camera& camera);

private:
    std::unique_ptr<Shader> shaderPhong;
    std::unique_ptr<Shader> shaderSemLuz;

    GLuint gradeVAO          = 0;
    GLuint gradeVBO          = 0;
    int    contadorVertGrade = 0;

    GLuint eixosVAO          = 0;
    GLuint eixosVBO          = 0;
    int    contadorVertEixos = 0;

    void construirGrade(int metadeExtensao, int passo);
    void construirEixos(float comprimento);

    void desenharGrade(const Camera& camera) const;
    void desenharEixos(const Camera& camera) const;

    void aplicarMaterial(Shader& sh, const Material&   mat) const;
    void aplicarLuz     (Shader& sh, const LuzPontual& luz) const;
};
