#include "SceneLoader.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <memory>

#include <nlohmann/json.hpp>

#include "Object3D.h"
#include "Model.h"
#include "Primitives.h"
#include "Curva.h"

using json = nlohmann::json;

// Funcoes auxiliares para ler tipos GLM do JSON -----------------------------

// Le um vec3 de um array JSON [x, y, z]; retorna padrao se o campo estiver ausente
static glm::vec3 lerVec3(const json& j, const std::string& chave, glm::vec3 padrao = glm::vec3(0.f)) {
    if (!j.contains(chave)) return padrao;
    const auto& v = j[chave];
    return { v[0].get<float>(), v[1].get<float>(), v[2].get<float>() };
}

// Le um float de um JSON; retorna padrao se ausente
static float lerFloat(const json& j, const std::string& chave, float padrao) {
    return j.contains(chave) ? j[chave].get<float>() : padrao;
}

// ---------------------------------------------------------------------------

void carregarCena(const std::string& caminho, Cena& cena, Camera& camera) {
    std::cout << "[SceneLoader] Carregando cena: " << caminho << "\n";

    std::ifstream arquivo(caminho);
    if (!arquivo.is_open()) {
        throw std::runtime_error("[SceneLoader] Nao foi possivel abrir: " + caminho);
    }

    json j;
    try {
        arquivo >> j;
    } catch (const json::exception& e) {
        throw std::runtime_error(std::string("[SceneLoader] Erro no JSON: ") + e.what());
    }

    // ---- Camera ----
    if (j.contains("camera")) {
        const auto& jc = j["camera"];
        camera.posicao        = lerVec3(jc, "posicao", camera.posicao);
        camera.guinada        = lerFloat(jc, "guinada",        camera.guinada);
        camera.arfagem        = lerFloat(jc, "arfagem",        camera.arfagem);
        camera.campoVisao     = lerFloat(jc, "campoVisao",     camera.campoVisao);
        camera.planoProximo   = lerFloat(jc, "planoProximo",   camera.planoProximo);
        camera.planoDistante  = lerFloat(jc, "planoDistante",  camera.planoDistante);
        camera.atualizarVetores();
        std::cout << "[SceneLoader] Camera configurada.\n";
    }

    // ---- Luz pontual ----
    if (j.contains("luz")) {
        const auto& jl = j["luz"];
        cena.luz.posicao    = lerVec3(jl, "posicao",   cena.luz.posicao);
        cena.luz.ambiente   = lerVec3(jl, "ambiente",  cena.luz.ambiente);
        cena.luz.difuso     = lerVec3(jl, "difuso",    cena.luz.difuso);
        cena.luz.especular  = lerVec3(jl, "especular", cena.luz.especular);
        cena.luz.constante  = lerFloat(jl, "constante",  cena.luz.constante);
        cena.luz.linear     = lerFloat(jl, "linear",     cena.luz.linear);
        cena.luz.quadratico = lerFloat(jl, "quadratico", cena.luz.quadratico);
        std::cout << "[SceneLoader] Luz configurada.\n";
    }

    // ---- Objetos ----
    if (!j.contains("objetos") || !j["objetos"].is_array()) {
        std::cerr << "[SceneLoader] Aviso: nenhum array 'objetos' encontrado.\n";
        return;
    }

    int carregados = 0;
    for (const auto& jo : j["objetos"]) {
        std::string nome = jo.contains("nome") ? jo["nome"].get<std::string>() : "objeto";

        std::shared_ptr<Modelo> modelo;

        // ---- Determina a origem do modelo: arquivo ou primitivo ----
        if (jo.contains("arquivo")) {
            std::string arquivoModelo = jo["arquivo"].get<std::string>();
            try {
                modelo = std::make_shared<Modelo>(arquivoModelo);
            } catch (const std::exception& e) {
                std::cerr << "[SceneLoader] Falha ao carregar '" << arquivoModelo
                          << "': " << e.what() << " — objeto ignorado.\n";
                continue;
            }
        } else if (jo.contains("primitivo")) {
            std::string tipo = jo["primitivo"].get<std::string>();
            modelo = std::make_shared<Modelo>();

            if (tipo == "cubo") {
                float tam = jo.contains("tamanho") ? jo["tamanho"].get<float>() : 1.f;
                modelo->malhas.push_back(Primitivos::criarCubo(tam));
            } else if (tipo == "esfera") {
                float raio    = jo.contains("raio")    ? jo["raio"].get<float>()    : 0.5f;
                int   setores = jo.contains("setores") ? jo["setores"].get<int>()   : 36;
                int   pilhas  = jo.contains("pilhas")  ? jo["pilhas"].get<int>()    : 24;
                modelo->malhas.push_back(Primitivos::criarEsfera(raio, setores, pilhas));
            } else if (tipo == "plano") {
                float tam = jo.contains("tamanho") ? jo["tamanho"].get<float>() : 1.f;
                modelo->malhas.push_back(Primitivos::criarPlano(tam));
            } else {
                std::cerr << "[SceneLoader] Primitivo desconhecido: '" << tipo << "' — ignorado.\n";
                continue;
            }
        } else {
            std::cerr << "[SceneLoader] Objeto '" << nome
                      << "' nao tem 'arquivo' nem 'primitivo' — ignorado.\n";
            continue;
        }

        auto obj = std::make_unique<Objeto3D>(nome, modelo);

        // ---- Transform inicial ----
        obj->posicao = lerVec3(jo, "posicao", glm::vec3(0.f));
        obj->rotacao = lerVec3(jo, "rotacao", glm::vec3(0.f));
        obj->escala  = lerVec3(jo, "escala",  glm::vec3(1.f));

        // ---- Material de substituicao (opcional) ----
        // Usado para primitivos ou para sobrepor o material do .mtl
        if (jo.contains("material")) {
            const auto& jm = jo["material"];

            // A cor de substituicao e aplicada a todas as malhas do modelo
            glm::vec3 cor = lerVec3(jm, "cor", glm::vec3(0.8f));
            for (auto& malha : obj->modelo->malhas) {
                if (jm.contains("cor"))       malha.material.cor       = cor;
                if (jm.contains("ambiente"))  malha.material.ambiente  = lerVec3(jm, "ambiente",  malha.material.ambiente);
                if (jm.contains("difuso"))    malha.material.difuso    = lerVec3(jm, "difuso",    malha.material.difuso);
                if (jm.contains("especular")) malha.material.especular = lerVec3(jm, "especular", malha.material.especular);
                if (jm.contains("brilho"))    malha.material.brilho    = lerFloat(jm, "brilho",   malha.material.brilho);
            }
        }

        // ---- Animacao de Bezier (opcional) ----
        if (jo.contains("animacao")) {
            const auto& ja = jo["animacao"];
            if (ja.contains("tipo") && ja["tipo"].get<std::string>() == "bezier") {
                if (ja.contains("pontos") && ja["pontos"].is_array() && ja["pontos"].size() == 4) {
                    CurvaBezier curva;
                    for (int pi = 0; pi < 4; ++pi) {
                        const auto& pt = ja["pontos"][pi];
                        curva.pontos[pi] = { pt[0].get<float>(), pt[1].get<float>(), pt[2].get<float>() };
                    }
                    curva.velocidade = lerFloat(ja, "velocidade", 0.3f);
                    curva.loop       = ja.contains("loop") ? ja["loop"].get<bool>() : true;
                    obj->animacao    = curva;
                    std::cout << "[SceneLoader] Animacao Bezier configurada para: " << nome << "\n";
                }
            }
        }

        cena.adicionarObjeto(std::move(obj));
        ++carregados;
    }

    std::cout << "[SceneLoader] " << carregados << " objetos carregados na cena.\n";
}
