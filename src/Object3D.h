#pragma once

#include <string>
#include <memory>
#include <optional>

#include <glm/glm.hpp>

#include "Model.h"
#include "Shader.h"
#include "Material.h"
#include "Curva.h"

// Objeto3D e um no da cena: combina um Modelo com um material de substituicao
// (usado em primitivos) e um transform TRS (translacao, rotacao, escala).
// Opcionalmente possui uma animacao de Bezier que move o objeto ao longo
// de uma curva parametrica a cada frame.
class Objeto3D {
public:
    std::string             nome;
    std::shared_ptr<Modelo> modelo;

    // Material de substituicao: aplicado quando o modelo nao tem material proprio
    // (primitivos) ou quando o SceneLoader define uma cor no JSON.
    // Se o Modelo ja tem materiais por malha (carregados do .mtl),
    // este campo e ignorado durante a renderizacao.
    Material material;

    glm::vec3 posicao = glm::vec3(0.f);
    glm::vec3 rotacao = glm::vec3(0.f);  // angulos de Euler em graus (X, Y, Z)
    glm::vec3 escala  = glm::vec3(1.f);

    bool estaSelecionado = false;

    // Animacao opcional: se definida, o objeto se move ao longo de uma curva de Bezier.
    std::optional<CurvaBezier> animacao;
    float tempoAnimacao = 0.f;  // parametro t atual da curva, em [0, 1]

    Objeto3D(const std::string& nome, std::shared_ptr<Modelo> modelo);

    // Constroi a matriz modelo TRS: translate -> rotateX -> rotateY -> rotateZ -> scale
    glm::mat4 obterMatrizModelo() const;

    // Faz upload da matriz modelo, normal matrix e flag de selecao,
    // depois delega o desenho para o Modelo (com material por malha).
    // Para primitivos (sem material por malha), aplica o material de substituicao
    // diretamente no shader antes de chamar o Modelo.
    void desenhar(Shader& shader) const;

    // Atualiza a posicao baseado na animacao de Bezier (se configurada).
    // Chamada a cada frame com o delta de tempo em segundos.
    void atualizar(float deltaTempo);

    void transladar(const glm::vec3& delta);
    void rotacionar(const glm::vec3& deltaDeg);
    void escalarPor(const glm::vec3& fatores);
    void escalarPor(float fatorUniforme);
    void resetarTransformacao();
};
