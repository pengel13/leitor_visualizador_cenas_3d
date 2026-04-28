#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include "Object3D.h"
#include "Scene.h"
#include "Renderer.h"
#include "Primitives.h"

static constexpr int   JANELA_L     = 1280;
static constexpr int   JANELA_A     = 720;
static constexpr char  TITULO_JANELA[] = "Visualizador 3D — OpenGL Moderno";

static Camera*        g_camera       = nullptr;
static Cena*          g_cena         = nullptr;
static Renderizador*  g_renderizador = nullptr;

static bool  g_primeiroMouse  = true;
static float g_ultimoX        = JANELA_L / 2.f;
static float g_ultimoY        = JANELA_A / 2.f;
static bool  g_mouseCapturado = true;

static bool g_teclas[GLFW_KEY_LAST + 1] = {};

static void callbackRedimensionar(GLFWwindow* /*win*/, int l, int a) {
    if (a == 0) return;
    glViewport(0, 0, l, a);
    if (g_camera) {
        g_camera->proporcaoTela = static_cast<float>(l) / static_cast<float>(a);
    }
}

static void callbackMoverMouse(GLFWwindow* /*win*/, double xpos, double ypos) {
    if (!g_camera || !g_mouseCapturado) return;

    float fx = static_cast<float>(xpos);
    float fy = static_cast<float>(ypos);

    if (g_primeiroMouse) {
        g_ultimoX = fx;
        g_ultimoY = fy;
        g_primeiroMouse = false;
    }

    float offsetX = fx - g_ultimoX;
    float offsetY = g_ultimoY - fy; // Y da tela e invertido

    g_ultimoX = fx;
    g_ultimoY = fy;

    g_camera->processarMovimentoMouse(offsetX, offsetY);
}

static void callbackRola(GLFWwindow* /*win*/, double /*x*/, double offsetY) {
    if (g_camera) {
        g_camera->processarRolaMouse(static_cast<float>(offsetY));
    }
}

static void callbackTecla(GLFWwindow* win, int tecla, int /*scan*/, int acao, int mods) {
    if (tecla >= 0 && tecla <= GLFW_KEY_LAST) {
        if (acao == GLFW_PRESS)   g_teclas[tecla] = true;
        if (acao == GLFW_RELEASE) g_teclas[tecla] = false;
    }

    if (acao != GLFW_PRESS) return;

    switch (tecla) {
        case GLFW_KEY_ESCAPE:
            if (g_mouseCapturado) {
                glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                g_mouseCapturado = false;
                g_primeiroMouse  = true;
            } else {
                glfwSetWindowShouldClose(win, GLFW_TRUE);
            }
            break;

        case GLFW_KEY_TAB:
            if (g_cena) {
                if (mods & GLFW_MOD_SHIFT) g_cena->selecionarAnterior();
                else                       g_cena->selecionarProximo();
            }
            break;

        case GLFW_KEY_P:
            if (g_camera) g_camera->alternarProjecao();
            std::cout << "[Camera] Projecao: "
                      << (g_camera->modoProjecao == ModoProjecao::Perspectiva
                          ? "Perspectiva" : "Ortografica") << "\n";
            break;

        case GLFW_KEY_F:
            if (g_renderizador) {
                g_renderizador->wireframeAtivado = !g_renderizador->wireframeAtivado;
                std::cout << "[Renderizador] Wireframe: "
                          << (g_renderizador->wireframeAtivado ? "ON" : "OFF") << "\n";
            }
            break;

        case GLFW_KEY_G:
            if (g_renderizador) {
                g_renderizador->mostrarGrade = !g_renderizador->mostrarGrade;
                std::cout << "[Renderizador] Grade: "
                          << (g_renderizador->mostrarGrade ? "ON" : "OFF") << "\n";
            }
            break;

        case GLFW_KEY_H:
            if (g_renderizador) {
                g_renderizador->mostrarEixos = !g_renderizador->mostrarEixos;
                std::cout << "[Renderizador] Eixos: "
                          << (g_renderizador->mostrarEixos ? "ON" : "OFF") << "\n";
            }
            break;

        case GLFW_KEY_L:
            if (g_cena) g_cena->imprimirStatus();
            break;

        case GLFW_KEY_BACKSPACE:
            if (g_cena) {
                Objeto3D* ativo = g_cena->obterAtivo();
                if (ativo) {
                    ativo->resetarTransformacao();
                    std::cout << "[Objeto3D] Transformacao resetada: " << ativo->nome << "\n";
                }
            }
            break;

        default: break;
    }
}

static void callbackBotaoMouse(GLFWwindow* win, int botao, int acao, int /*mods*/) {
    if (botao == GLFW_MOUSE_BUTTON_LEFT && acao == GLFW_PRESS && !g_mouseCapturado) {
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        g_mouseCapturado = true;
        g_primeiroMouse  = true;
    }
}

static void processarEntrada(float deltaTempo) {
    if (!g_camera || !g_cena) return;

    if (g_teclas[GLFW_KEY_W])     g_camera->processarTeclado(Camera::FRENTE,   deltaTempo);
    if (g_teclas[GLFW_KEY_S])     g_camera->processarTeclado(Camera::TRAS,     deltaTempo);
    if (g_teclas[GLFW_KEY_A])     g_camera->processarTeclado(Camera::ESQUERDA, deltaTempo);
    if (g_teclas[GLFW_KEY_D])     g_camera->processarTeclado(Camera::DIREITA,  deltaTempo);
    if (g_teclas[GLFW_KEY_SPACE]) g_camera->processarTeclado(Camera::CIMA,     deltaTempo);
    if (g_teclas[GLFW_KEY_C])     g_camera->processarTeclado(Camera::BAIXO,    deltaTempo);

    Objeto3D* obj = g_cena->obterAtivo();
    if (!obj) return;

    const float velTrans = 3.f * deltaTempo;
    const float velRot   = 90.f * deltaTempo;
    const float escCima  = 1.f + 0.8f * deltaTempo;
    const float escBaixo = 1.f / escCima;

    bool rPressionado = g_teclas[GLFW_KEY_R];

    if (!rPressionado) {
        if (g_teclas[GLFW_KEY_UP])        obj->transladar({ 0.f,  0.f, -velTrans});
        if (g_teclas[GLFW_KEY_DOWN])      obj->transladar({ 0.f,  0.f,  velTrans});
        if (g_teclas[GLFW_KEY_LEFT])      obj->transladar({-velTrans, 0.f,  0.f});
        if (g_teclas[GLFW_KEY_RIGHT])     obj->transladar({ velTrans, 0.f,  0.f});
        if (g_teclas[GLFW_KEY_PAGE_UP])   obj->transladar({ 0.f,  velTrans, 0.f});
        if (g_teclas[GLFW_KEY_PAGE_DOWN]) obj->transladar({ 0.f, -velTrans, 0.f});
    } else {
        if (g_teclas[GLFW_KEY_UP])     obj->rotacionar({-velRot, 0.f,   0.f});
        if (g_teclas[GLFW_KEY_DOWN])   obj->rotacionar({ velRot, 0.f,   0.f});
        if (g_teclas[GLFW_KEY_LEFT])   obj->rotacionar({ 0.f,  -velRot, 0.f});
        if (g_teclas[GLFW_KEY_RIGHT])  obj->rotacionar({ 0.f,   velRot, 0.f});
        if (g_teclas[GLFW_KEY_COMMA])  obj->rotacionar({ 0.f,   0.f, -velRot});
        if (g_teclas[GLFW_KEY_PERIOD]) obj->rotacionar({ 0.f,   0.f,  velRot});
    }

    if (g_teclas[GLFW_KEY_EQUAL] || g_teclas[GLFW_KEY_KP_ADD])      obj->escalarPor(escCima);
    if (g_teclas[GLFW_KEY_MINUS] || g_teclas[GLFW_KEY_KP_SUBTRACT]) obj->escalarPor(escBaixo);
}

static void criarCenaDemo(Cena& cena, const std::vector<std::string>& caminhos) {
    auto criarModeloPrimitivo = [](Malha&& malha) -> std::shared_ptr<Modelo> {
        auto m = std::make_shared<Modelo>();
        m->malhas.push_back(std::move(malha));
        return m;
    };

    {
        auto obj = std::make_unique<Objeto3D>(
            "Cubo",
            criarModeloPrimitivo(Primitivos::criarCubo(1.f))
        );
        obj->posicao          = glm::vec3(-2.f, 0.5f, 0.f);
        obj->material.cor     = glm::vec3(0.8f, 0.2f, 0.2f);
        obj->material.difuso  = glm::vec3(0.9f, 0.3f, 0.3f);
        obj->material.especular = glm::vec3(0.6f);
        obj->material.brilho  = 64.f;
        cena.adicionarObjeto(std::move(obj));
    }

    {
        auto obj = std::make_unique<Objeto3D>(
            "Esfera",
            criarModeloPrimitivo(Primitivos::criarEsfera(0.8f, 36, 24))
        );
        obj->posicao          = glm::vec3(2.f, 0.8f, 0.f);
        obj->material.cor     = glm::vec3(0.2f, 0.5f, 0.9f);
        obj->material.difuso  = glm::vec3(0.3f, 0.5f, 0.9f);
        obj->material.especular = glm::vec3(0.8f);
        obj->material.brilho  = 128.f;
        cena.adicionarObjeto(std::move(obj));
    }

    {
        auto obj = std::make_unique<Objeto3D>(
            "Chao",
            criarModeloPrimitivo(Primitivos::criarPlano(8.f))
        );
        obj->posicao          = glm::vec3(0.f);
        obj->material.cor     = glm::vec3(0.3f, 0.32f, 0.3f);
        obj->material.difuso  = glm::vec3(0.3f);
        obj->material.especular = glm::vec3(0.05f);
        obj->material.brilho  = 8.f;
        cena.adicionarObjeto(std::move(obj));
    }

    for (const auto& caminho : caminhos) {
        try {
            auto modelo = std::make_shared<Modelo>(caminho);
            auto obj    = std::make_unique<Objeto3D>(caminho, modelo);
            float offset = static_cast<float>(cena.contarObjetos()) * 3.f - 3.f;
            obj->posicao = glm::vec3(offset, 0.f, -4.f);
            cena.adicionarObjeto(std::move(obj));
        } catch (const std::exception& e) {
            std::cerr << "[Main] Falha ao carregar modelo '" << caminho << "': "
                      << e.what() << "\n";
        }
    }

    cena.luz.posicao = glm::vec3(4.f, 6.f, 4.f);
}

static void imprimirUso(const char* nomeExe) {
    std::cout
        << "┌─────────────────────────────────────────────┐\n"
        << "│       Visualizador 3D — OpenGL Moderno       │\n"
        << "└─────────────────────────────────────────────┘\n"
        << "Uso: " << nomeExe << " [modelo.obj] [modelo2.ply] ...\n\n"
        << "Controles:\n"
        << "  WASD / Space / C  — mover camera\n"
        << "  Mouse             — olhar ao redor\n"
        << "  TAB / Shift+TAB   — selecionar proximo/anterior\n"
        << "  Setas             — transladar objeto selecionado\n"
        << "  Page Up/Down      — transladar objeto para cima/baixo\n"
        << "  R + Setas         — rotacionar objeto selecionado\n"
        << "  R + , / .         — rotacionar em torno do eixo Z\n"
        << "  = / -             — escalar para cima / baixo\n"
        << "  P                 — alternar perspectiva/ortografica\n"
        << "  F                 — alternar wireframe\n"
        << "  G                 — alternar grade\n"
        << "  H                 — alternar eixos\n"
        << "  L                 — imprimir status da cena\n"
        << "  Backspace         — resetar transformacao do objeto\n"
        << "  Escape            — soltar mouse / sair\n\n";
}

int main(int argc, char** argv) {
    std::vector<std::string> caminhos;
    for (int i = 1; i < argc; ++i) {
        caminhos.emplace_back(argv[i]);
    }

    imprimirUso(argv[0]);

    if (!glfwInit()) {
        std::cerr << "[GLFW] Falha na inicializacao.\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* janela = glfwCreateWindow(JANELA_L, JANELA_A, TITULO_JANELA,
                                          nullptr, nullptr);
    if (!janela) {
        std::cerr << "[GLFW] Falha ao criar janela.\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(janela);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "[GLAD] Falha ao carregar ponteiros OpenGL.\n";
        glfwTerminate();
        return 1;
    }

    std::cout << "[OpenGL] Versao: " << glGetString(GL_VERSION) << "\n"
              << "[OpenGL] Renderizador: " << glGetString(GL_RENDERER) << "\n\n";

    glEnable(GL_MULTISAMPLE);

    Camera camera(glm::vec3(0.f, 2.f, 8.f),
                  static_cast<float>(JANELA_L) / static_cast<float>(JANELA_A));
    g_camera = &camera;

    glfwSetFramebufferSizeCallback(janela, callbackRedimensionar);
    glfwSetCursorPosCallback      (janela, callbackMoverMouse);
    glfwSetScrollCallback         (janela, callbackRola);
    glfwSetKeyCallback            (janela, callbackTecla);
    glfwSetMouseButtonCallback    (janela, callbackBotaoMouse);

    glfwSetInputMode(janela, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Cena         cena;
    g_cena       = &cena;

    Renderizador renderizador;
    g_renderizador = &renderizador;

    renderizador.inicializar();
    criarCenaDemo(cena, caminhos);

    std::cout << "[Cena] " << cena.contarObjetos() << " objetos carregados.\n"
              << "[Cena] Ativo: " << (cena.obterAtivo() ? cena.obterAtivo()->nome : "nenhum") << "\n\n";

    float ultimoTempo = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(janela)) {
        float tempoAtual = static_cast<float>(glfwGetTime());
        float deltaTempo = tempoAtual - ultimoTempo;
        ultimoTempo      = tempoAtual;

        // Limita para evitar saltos apos alt-tab ou breakpoints
        if (deltaTempo > 0.1f) deltaTempo = 0.1f;

        processarEntrada(deltaTempo);
        renderizador.renderizar(cena, camera);
        glfwSwapBuffers(janela);
        glfwPollEvents();
    }

    glfwDestroyWindow(janela);
    glfwTerminate();

    std::cout << "[Main] Ate mais.\n";
    return 0;
}
