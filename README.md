# Processamento Gráfico: Fundamentos 2026/1

Repositório com o projeto desenvolvido para a Atividade Acadêmica **Processamento Gráfico: Fundamentos**, parte do curso de graduação em **Ciência da Computação da Unisinos**.

👤 **Integrantes:**
- Lorenzo Kirst de Souza
- Pedro Engel

---

## 📂 Estrutura do Repositório

| Projeto | Engine / Linguagem | Descrição breve |
|---------|-------------------|-----------------|
| `Trabalho Grau B` | C++17 / OpenGL 4.6 | Leitor e visualizador de cenas 3D completo com cena configurável por JSON, texturas, animação por curva de Bézier e iluminação de Phong com materiais MTL |

---

## Leitor e Visualizador de Cenas 3D — Parte 2 (Grau B)

Aplicação desenvolvida em **C++17** com **OpenGL 4.6 Core Profile**. Carrega uma cena 3D completa definida em um arquivo `scene.json`, com múltiplos modelos OBJ+MTL, texturas difusas, iluminação de Phong por malha, câmera FPS configurável e animação de objetos por curva de Bézier cúbica.

### Funcionalidades Implementadas

| Requisito | Status | Descrição |
|-----------|--------|-----------|
| Leitura de múltiplos OBJ com grupos de malha | ✅ | Via Assimp — cada grupo é uma `Malha` independente |
| Normais e coordenadas de textura por vértice | ✅ | Extraídas pelo Assimp, usadas nos shaders |
| Textura difusa por malha (map_Kd do .mtl) | ✅ | stb_image + cache por caminho; fallback para cor do material |
| Propriedades ka, kd, ks, shininess do .mtl | ✅ | Lidas via Assimp (`AI_MATKEY_COLOR_*`) por malha |
| Iluminação de Phong (ambiente + difusa + especular) | ✅ | Shader GLSL por fragmento com atenuação quadrática |
| Fonte de luz pontual parametrizável | ✅ | Posição, intensidade, coeficientes de atenuação no JSON |
| Câmera FPS por teclado e mouse | ✅ | WASD + mouse, scroll para zoom |
| Câmera configurável via JSON | ✅ | Posição, yaw, pitch, FOV, near/far no `scene.json` |
| Seleção de objetos por teclado | ✅ | `TAB` / `SHIFT+TAB` — objeto ativo recebe tint amarelo |
| Rotação, translação, escala do objeto ativo | ✅ | Setas, R+setas, =/- |
| Objeto animado por curva de Bézier cúbica | ✅ | `OrbFlutuante` percorre loop na altura do olhar |
| Arquivo de configuração de cena (JSON) | ✅ | `scene.json` define objetos, luz, câmera e animações |
| ≥ 15 objetos na cena | ✅ | 19 objetos (museu virtual com paredes, pilares, pedestais, esferas) |
| Modo wireframe sobreposto | ✅ | Tecla `F` |
| Grade de chão e eixos XYZ | ✅ | Teclas `G` e `H` |
| Projeção perspectiva / ortográfica | ✅ | Tecla `P` |

---

## 🏛 Cena Padrão — Museu Virtual

A cena padrão (`scene.json`) representa uma sala de museu com:

- **Estrutura:** chão, teto e 4 paredes brancas
- **4 pilares** interiores em mármore cinza
- **4 pedestais** de exposição brancos
- **4 esferas coloridas** sobre os pedestais (vermelho, verde, azul, dourado)
- **Cubo decorativo central** rotacionado
- **OrbFlutuante** — esfera dourada animada por curva de Bézier, que percorre um caminho oval em torno do museu continuamente

Para adicionar seus próprios modelos OBJ, coloque os arquivos em `assets/models/` e referencie-os no `scene.json` com o campo `"arquivo"`.

---

## 📄 Arquivo de Configuração de Cena (`scene.json`)

O arquivo `scene.json` deve estar no **diretório de trabalho do executável** (normalmente `build/Release/` ou `build/Debug/`). O CMake copia automaticamente o arquivo após o build.

### Estrutura do JSON

```json
{
  "camera": {
    "posicao":      [x, y, z],
    "guinada":      -90.0,
    "arfagem":      0.0,
    "campoVisao":   45.0,
    "planoProximo": 0.1,
    "planoDistante": 500.0
  },

  "luz": {
    "posicao":    [x, y, z],
    "ambiente":   [r, g, b],
    "difuso":     [r, g, b],
    "especular":  [r, g, b],
    "constante":  1.0,
    "linear":     0.045,
    "quadratico": 0.0075
  },

  "objetos": [
    {
      "nome":      "NomeDoObjeto",
      "arquivo":   "assets/models/meumodelo.obj",
      "posicao":   [x, y, z],
      "rotacao":   [rx, ry, rz],
      "escala":    [sx, sy, sz],
      "material": {
        "cor":       [r, g, b],
        "difuso":    [r, g, b],
        "especular": [r, g, b],
        "brilho":    64.0
      }
    },
    {
      "nome":      "Primitivo",
      "primitivo": "cubo",
      "tamanho":   1.0,
      "posicao":   [x, y, z],
      "escala":    [sx, sy, sz]
    },
    {
      "nome":      "ObjAnimado",
      "primitivo": "esfera",
      "posicao":   [0, 2, 0],
      "animacao": {
        "tipo":       "bezier",
        "pontos":     [[x0,y0,z0], [x1,y1,z1], [x2,y2,z2], [x3,y3,z3]],
        "velocidade": 0.15,
        "loop":       true
      }
    }
  ]
}
```

**Tipos de primitivos suportados:** `"cubo"`, `"esfera"`, `"plano"`

**Campos opcionais por objeto:** `"material"` (sobrescreve o .mtl), `"animacao"` (somente tipo `"bezier"`)

---

## 🛠 Pré-requisitos

| Dependência | Versão mínima | Finalidade |
|-------------|---------------|------------|
| CMake | ≥ 3.20 | Sistema de build |
| vcpkg | qualquer | Gerenciador de pacotes |
| GLAD | GL 4.6 core | Loader de funções OpenGL |
| GLFW3 | ≥ 3.3 | Janela e entrada |
| GLM | ≥ 0.9.9 | Álgebra linear (vetores, matrizes) |
| Assimp | ≥ 5.0 | Importação de modelos 3D e leitura de .mtl |
| nlohmann/json | ≥ 3.10 | Leitura do arquivo de configuração de cena |
| stb | (stb_image) | Carregamento de texturas (PNG, JPG, BMP) |
| Compilador C++ | C++17 | MSVC 2019+, GCC 9+, Clang 9+ |

---

## 🔧 Instruções de Compilação

### Windows (MSVC + vcpkg) — Recomendado

**Passo 1 — Instalar o vcpkg** (pule se já tiver)

```bat
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

**Passo 2 — Configurar e compilar** (vcpkg instala as dependências automaticamente via manifest mode)

```bat
cmake -S . -B build ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release
```

**Passo 3 — Executar**

```bat
cd build\Release
SceneViewer3D.exe
```

O executável procura `scene.json`, `shaders/` e `assets/` no diretório atual. O CMake copia esses arquivos automaticamente após o build.

### Linux (GCC/Clang + vcpkg)

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=<vcpkg_root>/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build
./build/SceneViewer3D
```

---

## ▶ Modos de Execução

### Modo normal — cena JSON (recomendado)

Coloque `scene.json` no diretório do executável e execute sem argumentos:

```bat
.\build\Release\SceneViewer3D.exe
```

### Modo fallback — argumentos de linha de comando

Se `scene.json` não estiver presente, os modelos passados como argumentos são carregados em uma cena de demonstração simples:

```bat
.\build\Release\SceneViewer3D.exe assets\models\meumodelo.obj
```

### Adicionando texturas

Coloque os arquivos `.png` ou `.jpg` referenciados no `.mtl` junto com o `.obj` em `assets/models/`. O carregador tenta primeiro o caminho absoluto do `.mtl` e depois o diretório do modelo como prefixo.

---

## 🎮 Controles

### Câmera

| Tecla / Entrada | Ação |
|-----------------|------|
| `W A S D` | Mover frente / esquerda / trás / direita |
| `Space` | Subir câmera |
| `C` | Descer câmera |
| `Mouse` | Olhar (yaw/pitch) |
| `Scroll` | Zoom (altera FOV) |
| `P` | Alternar **Perspectiva ↔ Ortográfica** |
| `Escape` | Liberar cursor / fechar janela |
| Clique esquerdo | Recapturar cursor |

### Seleção de Objetos

| Tecla | Ação |
|-------|------|
| `TAB` | Selecionar **próximo** objeto |
| `SHIFT + TAB` | Selecionar **objeto anterior** |

O objeto selecionado recebe um leve tint amarelo-branco.

### Transformações (objeto selecionado)

| Tecla(s) | Modo | Ação |
|----------|------|------|
| `↑ ↓` | Translação | Mover em **−Z / +Z** |
| `← →` | Translação | Mover em **−X / +X** |
| `Page Up / Page Down` | Translação | Mover em **+Y / −Y** |
| `R + ↑ ↓` | Rotação | Eixo **X** |
| `R + ← →` | Rotação | Eixo **Y** |
| `R + , .` | Rotação | Eixo **Z** |
| `=` ou `KP+` | Escala | **Aumentar** (uniforme) |
| `-` ou `KP-` | Escala | **Diminuir** (uniforme) |
| `Backspace` | Transformação | **Resetar** para o estado inicial |

### Modos de Renderização

| Tecla | Ação |
|-------|------|
| `F` | Alternar wireframe sobreposto |
| `G` | Alternar grade de chão |
| `H` | Alternar eixos XYZ |
| `L` | Imprimir estado da cena no console |

---

## 🏗 Estrutura do Projeto

```
leitor_visualizador_cenas_3d/
│
├── CMakeLists.txt            # Script de build
├── vcpkg.json                # Manifesto de dependências vcpkg
├── scene.json                # Cena padrão (museu virtual, 19 objetos)
├── README.md                 # Este arquivo
│
├── src/
│   ├── main.cpp              # Ponto de entrada: GLFW, GLAD, loop, input
│   ├── Shader.h/.cpp         # Compilação/link GLSL, cache de uniforms
│   ├── Camera.h/.cpp         # Câmera FPS, matrizes view/projection
│   ├── Material.h            # Struct Material (ka, kd, ks, shininess, cor)
│   ├── Mesh.h/.cpp           # VAO/VBO/EBO; desenhar() e desenhar(Shader&)
│   ├── Model.h/.cpp          # Assimp loader; leitura de .mtl e texturas
│   ├── Texture.h/.cpp        # stb_image loader com cache por caminho
│   ├── Object3D.h/.cpp       # Transform TRS, animação de Bézier, draw
│   ├── Curva.h/.cpp          # Struct CurvaBezier + avaliarBezier(t)
│   ├── Scene.h/.cpp          # Lista de objetos, seleção, atualizar()
│   ├── Renderer.h/.cpp       # Passes de renderização (Phong, wireframe, grade)
│   ├── SceneLoader.h/.cpp    # Parser JSON → Cena + Camera
│   └── Primitives.h/.cpp     # Geometria procedural (cubo, esfera, plano)
│
├── shaders/
│   ├── phong.vert            # Vertex: transform MVP + normais (normalMatrix)
│   ├── phong.frag            # Fragment: Phong com textura difusa e atenuação
│   ├── unlit.vert            # Vertex: MVP simples (grade, eixos, wireframe)
│   └── unlit.frag            # Fragment: cor por vértice × tint uniform
│
└── assets/
    └── models/               # Modelos .obj, .mtl e texturas (.png/.jpg)
```

---

## 📚 Referências

- [Learn OpenGL — tutoriais de OpenGL moderna](https://learnopengl.com/)
- [OpenGL Reference Pages — documentação oficial](https://registry.khronos.org/OpenGL-Refpages/gl4/)
- [GLFW Documentation](https://www.glfw.org/docs/latest/)
- [GLM Manual](https://glm.g-truc.net/0.9.9/api/index.html)
- [Assimp Documentation](https://assimp-docs.readthedocs.io/)
- [nlohmann/json](https://github.com/nlohmann/json)
- [stb_image](https://github.com/nothings/stb)
