#pragma once

#include <memory>
#include <glad/glad.h>
#include "Scene.h"
#include "Camera.h"
#include "Shader.h"

// Renderizador centraliza todos os passes de renderizacao:
//   1. Passo Phong: iluminacao com materiais e texturas por malha
//   2. Passo wireframe (opcional): overlay de arestas sobre os objetos
//   3. Grade e eixos de coordenadas (opcionais, para navegacao)
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

    // Inicializa shaders, constroi grade e eixos, habilita depth test
    void inicializar();

    // Executa todos os passes de renderizacao para um frame
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

    // Gera a geometria da grade no plano XZ
    void construirGrade(int metadeExtensao, int passo);

    // Gera os tres eixos de coordenadas coloridos (X=vermelho, Y=verde, Z=azul)
    void construirEixos(float comprimento);

    void desenharGrade(const Camera& camera) const;
    void desenharEixos(const Camera& camera) const;

    // Faz upload das propriedades da luz pontual para o shader
    void aplicarLuz(Shader& sh, const LuzPontual& luz) const;
};
