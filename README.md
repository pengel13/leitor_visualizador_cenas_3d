# Processamento Gráfico: Fundamentos 2026/1

Repositório com o projeto desenvolvido para a Atividade Acadêmica **Processamento Gráfico: Fundamentos**, parte do curso de graduação em **Ciência da Computação da Unisinos**.

👤 **Integrantes:**
- Lorenzo Kirst de Souza
- Pedro Engel

---

## 📂 Estrutura do Repositório

| Projeto | Engine / Linguagem | Descrição breve |
|---------|-------------------|-----------------|
| `Trabalho Grau A` | C++17 / OpenGL 4.6 | Leitor e visualizador de cenas 3D com iluminação de Phong, câmera FPS e transformações interativas |

---

## Leitor e Visualizador de Cenas 3D — Parte 1

Aplicação desenvolvida em **C++17** com **OpenGL 4.6 Core Profile**. Carrega múltiplos modelos 3D simultaneamente, renderiza com **iluminação de Phong** e oferece controle de câmera em primeira pessoa com transformações interativas por objeto.

### Funcionalidades Implementadas

| Requisito | Status | Descrição |
|-----------|--------|-----------|
| Parser `.obj` / `.ply` | ✅ | Via biblioteca Assimp (vértices, faces, normais, UVs) |
| VAO + VBO + EBO | ✅ | Pipeline de buffers explícito com `glVertexAttribPointer` |
| Múltiplos objetos na cena | ✅ | N objetos carregados e exibidos simultaneamente |
| Seleção por teclado | ✅ | `TAB` / `SHIFT+TAB` para alternar objeto ativo |
| Rotação X, Y, Z | ✅ | Tecla `R` + setas e vírgula/ponto |
| Translação X, Y, Z | ✅ | Setas + Page Up / Page Down |
| Escala uniforme | ✅ | Teclas `+` / `-` |
| Câmera FPS | ✅ | WASD + mouse |
| Projeção Perspectiva / Ortográfica | ✅ | Tecla `P` |
| Modelo de Phong (Ambiente + Difusa + Especular) | ✅ | Implementado nos shaders GLSL |
| Fonte de luz pontual configurável | ✅ | Com atenuação quadrática |
| Propriedades ka, kd, ks configuráveis | ✅ | Via `struct Material` em `Object3D` |
| Modo wireframe sobreposto | ✅ | Tecla `F` |
| Grid de chão e eixos (extra) | ✅ | Teclas `G` e `H` |

---

## 🛠 Pré-requisitos

| Dependência | Versão mínima | Finalidade |
|-------------|---------------|------------|
| CMake | ≥ 3.20 | Sistema de build |
| GLAD | GL 4.6 core | Loader de funções OpenGL |
| GLFW3 | ≥ 3.3 | Janela e entrada |
| GLM | ≥ 0.9.9 | Álgebra linear (vetores, matrizes) |
| Assimp | ≥ 5.0 | Importação de modelos 3D |
| Compilador C++ | C++17 | MSVC 2019+, GCC 9+, Clang 9+ |

---

## 🔧 Instruções de Compilação

### Windows (MSVC + vcpkg) — Recomendado

**Passo 1 — Instalar o vcpkg** (pule se já tiver)


git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
Passo 2 — Instalar as dependências


C:\vcpkg\vcpkg install glad glfw3 glm assimp --triplet x64-windows
Se o arquivo vcpkg.json estiver na raiz do projeto, o vcpkg instala as dependências automaticamente durante o cmake (manifest mode).

Passo 3 — Configurar e compilar


git clone https://github.com/pengel13/leitor_visualizador_cenas_3d.git
cd leitor_visualizador_cenas_3d

cmake -S . -B build ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release
Passo 4 — Executar


.\build\Release\SceneViewer3D.exe
Linux (GCC/Clang)

# Ubuntu / Debian
sudo apt install cmake libglfw3-dev libglm-dev libassimp-dev

git clone https://github.com/pengel13/leitor_visualizador_cenas_3d.git
cd leitor_visualizador_cenas_3d

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/SceneViewer3D
Nota: O GLAD já está incluído no projeto. No Linux é necessário ter o driver OpenGL com suporte à versão 4.6.

▶ Exemplo de Uso
Executar com modelos padrão da cena de demonstração:


.\build\Release\SceneViewer3D.exe
Carregar modelos via argumento de linha de comando:


.\build\Release\SceneViewer3D.exe assets\models\cogumelo.obj assets\models\outro.obj
Adicionar objetos programaticamente — edite createDemoScene() em src/main.cpp:


auto model = std::make_shared<Model>("assets/models/meu_modelo.obj");
auto obj   = std::make_unique<Object3D>("Meu Objeto", model);

obj->position         = glm::vec3(0.f, 0.f, 0.f);
obj->scale            = glm::vec3(1.5f);
obj->material.color   = glm::vec3(0.8f, 0.2f, 0.2f); // vermelho
obj->material.shininess = 64.f;

scene.addObject(std::move(obj));
🎮 Controles
Câmera
Tecla / Entrada	Ação
W A S D	Mover frente / esquerda / trás / direita
Space	Subir câmera
C	Descer câmera
Mouse	Olhar (yaw/pitch)
Scroll	Zoom (altera FOV)
P	Alternar Perspectiva ↔ Ortográfica
Escape	Liberar cursor do mouse
Clique esquerdo	Recapturar cursor
Seleção de Objetos
Tecla	Ação
TAB	Selecionar próximo objeto
SHIFT + TAB	Selecionar objeto anterior
Transformações (objeto selecionado)
Tecla(s)	Modo	Ação
↑ ↓	Translação	Mover em −Z / +Z
← →	Translação	Mover em −X / +X
Page Up / Page Down	Translação	Mover em +Y / −Y
R + ↑ ↓	Rotação	Rotacionar em torno do eixo X
R + ← →	Rotação	Rotacionar em torno do eixo Y
R + , .	Rotação	Rotacionar em torno do eixo Z
= ou +	Escala	Aumentar escala uniforme
-	Escala	Diminuir escala uniforme
Backspace	Transformação	Resetar para identidade
Modos de Renderização
Tecla	Ação
F	Alternar wireframe sobreposto
G	Alternar grid de chão
H	Alternar eixos XYZ
L	Imprimir estado da cena no console
🏗 Estrutura do Projeto
'''
leitor_visualizador_cenas_3d/
│
├── CMakeLists.txt          # Script de build
├── vcpkg.json              # Manifesto de dependências vcpkg
├── README.md               # Este arquivo
│
├── src/
│   ├── main.cpp            # Ponto de entrada: GLFW, GLAD, loop, input
│   ├── Shader.h/.cpp       # Compilação/link GLSL, cache de uniforms
│   ├── Camera.h/.cpp       # Câmera FPS, matrizes view/projection
│   ├── Mesh.h/.cpp         # VAO/VBO/EBO, glVertexAttribPointer
│   ├── Model.h/.cpp        # Carregador Assimp → vector<Mesh>
│   ├── Object3D.h/.cpp     # Transform, Material, chamada de draw
│   ├── Scene.h/.cpp        # Lista de objetos, seleção ativa, luz
│   ├── Renderer.h/.cpp     # Loop de renderização, grid/eixos
│   └── Primitives.h/.cpp   # Geometria procedural (cubo, esfera, plano)
│
├── shaders/
│   ├── phong.vert          # Vertex:   transform MVP + normais
│   ├── phong.frag          # Fragment: Phong ambiente+difusa+especular
│   ├── unlit.vert          # Vertex:   MVP simples (sem iluminação)
│   └── unlit.frag          # Fragment: cor por vértice × tint uniform
│
└── assets/
    └── models/             # Modelos 3D de exemplo (.obj, .ply)
'''

📚 Referências
Learn OpenGL — tutoriais de OpenGL moderna
Anton's OpenGL 4 Tutorials — guia detalhado GL4
OpenGL Reference Pages — documentação oficial
GLFW Documentation
GLM Manual
Assimp Documentation
