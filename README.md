# 3D Scene Viewer — Modern OpenGL

A fully featured **3D scene viewer** built with **C++17** and **Modern OpenGL 4.6 Core Profile**.  
Loads multiple 3D models simultaneously, renders them with **Phong lighting**, and provides an interactive **FPS-style camera** with full object transform controls.

---

## Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Prerequisites](#prerequisites)
- [Build Instructions](#build-instructions)
  - [Windows (MSVC + vcpkg)](#windows-msvc--vcpkg)
  - [Linux (GCC/Clang)](#linux-gccclang)
  - [macOS](#macos)
- [Controls](#controls)
- [Shader Pipeline](#shader-pipeline)
- [Project Structure](#project-structure)
- [Adding Models to the Scene](#adding-models-to-the-scene)
- [Customising Materials and Lighting](#customising-materials-and-lighting)
- [Recommended 3D Model Sources](#recommended-3d-model-sources)
- [Recommended Tools](#recommended-tools)
- [Troubleshooting](#troubleshooting)
- [References](#references)

---

## Features

| Feature | Details |
|---------|---------|
| **Geometry Loading** | Assimp-powered loader for `.obj`, `.ply`, `.fbx`, `.gltf`, `.dae` and more |
| **GPU Buffers** | VAO + VBO + EBO pipeline with explicit `glVertexAttribPointer` setup |
| **Phong Lighting** | Full ambient + diffuse + specular with point-light attenuation |
| **FPS Camera** | Mouse look (yaw/pitch), WASD movement, perspective/orthographic toggle |
| **Object Selection** | TAB-cycle through objects; active object highlighted |
| **Object Transforms** | Translate (XYZ), rotate (XYZ Euler), scale (uniform) per selected object |
| **Wireframe Overlay** | Toggle a second render pass in `GL_LINE` mode |
| **Ground Grid** | Infinite-looking XZ grid with coloured cardinal lines |
| **Coordinate Axes** | RGB-coded XYZ world axes (X=red, Y=green, Z=blue) |
| **Procedural Geometry** | Built-in cube, UV sphere, and plane — no external files needed |
| **Modern OpenGL** | Core profile 4.6 — zero fixed-function pipeline |

---

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                       main.cpp                      │
│   GLFW window, GLAD init, callbacks, main loop      │
└────────────┬───────────────────┬────────────────────┘
             │                   │
      ┌──────▼──────┐     ┌──────▼──────┐
      │   Camera    │     │    Scene    │
      │  FPS view   │     │ objects[]   │
      │  matrices   │     │ PointLight  │
      └─────────────┘     └──────┬──────┘
                                 │  owns N×
                          ┌──────▼──────────┐
                          │    Object3D     │
                          │  position/rot   │
                          │  scale/Material │
                          └──────┬──────────┘
                                 │  shares
                          ┌──────▼──────────┐
                          │     Model       │
                          │ Assimp loader   │
                          └──────┬──────────┘
                                 │  owns N×
                          ┌──────▼──────────┐
                          │     Mesh        │
                          │  VAO/VBO/EBO    │
                          └─────────────────┘

             ┌─────────────────────────────┐
             │          Renderer           │
             │  phongShader  unlitShader   │
             │  grid VAO     axes VAO      │
             │  render(scene, camera)      │
             └─────────────────────────────┘

             ┌─────────────────────────────┐
             │          Shader             │
             │  compile/link GLSL          │
             │  uniform cache              │
             └─────────────────────────────┘
```

### CPU → GPU Data Flow

```
std::vector<Vertex>   ──glBufferData──►  VBO (GL_ARRAY_BUFFER)
std::vector<uint>     ──glBufferData──►  EBO (GL_ELEMENT_ARRAY_BUFFER)
glVertexAttribPointer ──describes──►     VAO (records layout)

Per frame:
glm::mat4 model       ──glUniform──►  GPU uniform block
glm::mat4 view        ──glUniform──►  GPU uniform block
glm::mat4 projection  ──glUniform──►  GPU uniform block
Material coefficients ──glUniform──►  GPU uniform block
```

---

## Prerequisites

| Dependency | Version | Purpose |
|-----------|---------|---------|
| **CMake** | ≥ 3.20 | Build system |
| **GLAD** | GL 4.6 core | OpenGL function loader |
| **GLFW3** | ≥ 3.3 | Window + input |
| **GLM** | ≥ 0.9.9 | Math (vectors, matrices) |
| **Assimp** | ≥ 5.0 | 3D model import |
| **C++ compiler** | C++17 | MSVC 2019+, GCC 9+, Clang 9+ |

The easiest way to manage these on Windows is **vcpkg**.

---

## Build Instructions

### Windows (MSVC + vcpkg)

**Step 1 — Install vcpkg** (skip if you already have it)

```bat
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

**Step 2 — Install dependencies**

```bat
C:\vcpkg\vcpkg install glad glfw3 glm assimp --triplet x64-windows
```

> Alternatively, if you keep `vcpkg.json` in the project root and use the manifest mode,
> vcpkg will install dependencies automatically during the CMake configure step.

**Step 3 — Configure and build**

```bat
cd leitor_visualizador_cenas_3d

cmake -S . -B build ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release
```

**Step 4 — Run**

```bat
.\build\Release\SceneViewer3D.exe
```

Or load a model:

```bat
.\build\Release\SceneViewer3D.exe assets\models\cube.obj
```

---

### Linux (GCC/Clang)

```bash
# Ubuntu / Debian
sudo apt install cmake libglfw3-dev libglm-dev libassimp-dev

# Install GLAD via pip (generates the loader)
pip install glad
glad --api gl:core=4.6 --out-path glad_output --generator c
# Copy glad_output/src/glad.c → src/glad.c
# Copy glad_output/include/ → include/

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/SceneViewer3D
```

---

### macOS

```bash
brew install cmake glfw glm assimp

# macOS only supports OpenGL 4.1 — change GLFW hints in main.cpp:
#   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
# Also change #version 460 core → #version 410 core in all .glsl files.

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/SceneViewer3D
```

---

## Controls

### Camera

| Key / Input | Action |
|------------|--------|
| `W` `A` `S` `D` | Move forward / left / backward / right |
| `Space` | Move camera up |
| `C` | Move camera down |
| Mouse move | Look around (yaw/pitch) |
| Scroll wheel | Zoom (change FOV) |
| `P` | Toggle **Perspective ↔ Orthographic** |
| `Escape` | Release mouse cursor |
| Left-click | Recapture mouse cursor |

### Object Selection

| Key | Action |
|-----|--------|
| `Tab` | Select **next** object |
| `Shift + Tab` | Select **previous** object |

The active object is highlighted with a subtle tint.

### Object Transforms (apply to selected object)

| Key(s) | Mode | Action |
|--------|------|--------|
| `↑` `↓` | Translate | Move along **−Z / +Z** |
| `←` `→` | Translate | Move along **−X / +X** |
| `Page Up` | Translate | Move **up (+Y)** |
| `Page Down` | Translate | Move **down (−Y)** |
| `R` + `↑` `↓` | Rotate | Rotate around **X axis** |
| `R` + `←` `→` | Rotate | Rotate around **Y axis** |
| `R` + `,` `.` | Rotate | Rotate around **Z axis** |
| `=` or `+` (numpad) | Scale | **Scale up** (uniform) |
| `-` or `-` (numpad) | Scale | **Scale down** (uniform) |
| `Backspace` | Transform | **Reset** to identity |

### Rendering Toggles

| Key | Action |
|-----|--------|
| `F` | Toggle **wireframe overlay** |
| `G` | Toggle **ground grid** |
| `H` | Toggle **XYZ axis lines** |
| `L` | Print **scene status** to console |

---

## Shader Pipeline

### Phong Vertex Shader (`shaders/phong.vert`)

Transforms each vertex through the MVP matrix chain and passes
world-space data to the fragment shader:

```
aPos (object space)
  │
  × model matrix          → v_fragPos  (world space)
  × normalMatrix          → v_normal   (world space, non-uniform-scale safe)
  │
  × view × projection     → gl_Position (clip space)
```

The **normal matrix** `= transpose(inverse(mat3(model)))` ensures normals
remain perpendicular to the surface even when the model matrix applies
non-uniform scale.

### Phong Fragment Shader (`shaders/phong.frag`)

Evaluates the Phong reflection model per-fragment:

```glsl
// Vectors
N = normalize(v_normal)             // surface normal
L = normalize(light_pos - fragPos)  // to light
V = normalize(viewPos - fragPos)    // to camera
R = reflect(-L, N)                  // reflection of light

// Attenuation
d   = length(light_pos - fragPos)
att = 1.0 / (Kc + Kl*d + Kq*d²)

// Components
ambient  = La × Ka × albedo
diffuse  = Ld × max(dot(N,L), 0) × Kd × albedo
specular = Ls × pow(max(dot(V,R), 0), shininess) × Ks

result   = (ambient + diffuse + specular) × att
```

### Unlit Shader (`shaders/unlit.vert` + `unlit.frag`)

Simple pass-through shader that multiplies per-vertex colour
by a uniform tint. Used for grid, axes and wireframe overlay.

---

## Project Structure

```
leitor_visualizador_cenas_3d/
│
├── CMakeLists.txt          # CMake build script
├── vcpkg.json              # vcpkg manifest (auto-installs deps)
├── README.md               # This file
│
├── src/
│   ├── main.cpp            # Entry point: GLFW, GLAD, loop, input
│   ├── Shader.h/.cpp       # GLSL compile/link, uniform cache
│   ├── Camera.h/.cpp       # FPS camera, view/projection matrices
│   ├── Mesh.h/.cpp         # VAO/VBO/EBO, glVertexAttribPointer
│   ├── Model.h/.cpp        # Assimp model loader → vector<Mesh>
│   ├── Object3D.h/.cpp     # Transform, Material, draw call
│   ├── Scene.h/.cpp        # Object list, active selection, light
│   ├── Renderer.h/.cpp     # Per-frame draw loop, grid/axes
│   └── Primitives.h/.cpp   # Procedural cube, sphere, plane
│
├── shaders/
│   ├── phong.vert          # Vertex:   MVP transform + normals
│   ├── phong.frag          # Fragment: Phong ambient+diffuse+specular
│   ├── unlit.vert          # Vertex:   simple MVP (no lighting)
│   └── unlit.frag          # Fragment: per-vertex color × uniform tint
│
└── assets/
    └── models/
        ├── cube.obj         # Sample cube mesh for testing
        ├── cube.mtl         # Sample material file
        └── README.txt       # Model format tips
```

---

## Adding Models to the Scene

**Via command line** (loads automatically, placed at Z = -4):

```bat
SceneViewer3D.exe assets\models\my_model.obj assets\models\another.ply
```

**Programmatically** — edit `createDemoScene()` in `src/main.cpp`:

```cpp
// Load a model file
auto model = std::make_shared<Model>("assets/models/my_model.obj");
auto obj   = std::make_unique<Object3D>("My Model", model);

// Set initial position, rotation, scale
obj->position = glm::vec3(0.f, 0.f, 0.f);
obj->scale    = glm::vec3(2.f);               // 2× scale

// Set Phong material
obj->material.color     = glm::vec3(0.2f, 0.7f, 0.3f); // green
obj->material.shininess = 64.f;

scene.addObject(std::move(obj));
```

---

## Customising Materials and Lighting

### Material (in `src/Object3D.h`)

```cpp
struct Material {
    glm::vec3 ambient   = glm::vec3(0.1f);         // ka
    glm::vec3 diffuse   = glm::vec3(0.8f);         // kd
    glm::vec3 specular  = glm::vec3(0.5f);         // ks
    float     shininess = 32.f;                    // Phong exponent
    glm::vec3 color     = glm::vec3(0.8f);         // base albedo
};
```

### Light (in `src/Scene.h`)

```cpp
// In createDemoScene() — src/main.cpp
scene.light.position  = glm::vec3(4.f, 6.f, 4.f);
scene.light.diffuse   = glm::vec3(1.0f);           // white
scene.light.ambient   = glm::vec3(0.15f);          // dim fill
scene.light.linear    = 0.045f;                    // attenuation
scene.light.quadratic = 0.0075f;
```

---

## Recommended 3D Model Sources

| Site | Description |
|------|-------------|
| [Poly Pizza](https://poly.pizza/) | Free low-poly models, great for testing |
| [Sketchfab](https://sketchfab.com/) | Large community library — use "Downloadable" filter |
| [Poly Haven](https://polyhaven.com/) | High-quality PBR models and textures |
| [AmbientCG](https://ambientcg.com/) | PBR material textures |
| [Free3D](https://free3d.com/) | Various formats including .obj |

**Export tips from Blender:**
1. File → Export → Wavefront (.obj)
2. Enable **Triangulate Faces**
3. Enable **Include Normals**
4. Enable **Include UVs**
5. Set **Forward = -Z, Up = Y** (OpenGL convention)

---

## Recommended Tools

| Tool | Use |
|------|-----|
| [Blender](https://www.blender.org/) | Model creation, conversion, UV unwrapping |
| [MeshLab](https://www.meshlab.net/) | Clean up meshes, reduce polygon count, format conversion |
| [RenderDoc](https://renderdoc.org/) | GPU debugger — inspect VAOs, buffers, draw calls |
| [Nsight Graphics](https://developer.nvidia.com/nsight-graphics) | NVIDIA GPU profiler and frame debugger |
| [GLAD generator](https://glad.dav1d.de/) | Generate GLAD loader for any OpenGL version |

---

## Troubleshooting

**"Cannot open shader file"**  
Make sure the `shaders/` directory is in the same folder as the executable.  
The CMake post-build step copies it automatically, but manual runs need the right working directory.

**Black screen / nothing visible**  
- Check the console for OpenGL error messages.  
- Try pressing `L` to print scene status and verify objects are loaded.  
- Make sure depth test is enabled (`glEnable(GL_DEPTH_TEST)` in `Renderer::init()`).

**Models load but appear very small or giant**  
Many real-world models use different unit scales.  
Use the `=` / `-` keys to scale the selected object, or set `obj->scale` in `createDemoScene()`.

**Mouse cursor doesn't move the camera**  
Click inside the window to recapture the cursor after pressing `Escape`.

**GLAD fails to initialise**  
Ensure your graphics driver supports OpenGL 4.6.  
Check with `glxinfo | grep "OpenGL version"` (Linux) or [GPU Caps Viewer](https://www.ozone3d.net/gpu_caps_viewer/) (Windows).  
If you only have 4.1 (macOS), change `GLFW_CONTEXT_VERSION_MINOR` to `1` and update `#version 460` → `#version 410` in all shaders.

**Assimp error loading .obj**  
Ensure the `.mtl` file (if referenced) is in the same directory as the `.obj`.  
Use Blender or MeshLab to re-export and triangulate before loading.

---

## References

- [Learn OpenGL](https://learnopengl.com/) — comprehensive modern OpenGL tutorials
- [Anton's OpenGL 4 Tutorials](https://antongerdelan.net/opengl/) — detailed GL4 guide
- [OpenGL Tutorial](http://www.opengl-tutorial.org/) — classic OpenGL tutorial series
- [OpenGL Reference Pages](https://registry.khronos.org/OpenGL-Refpages/gl4/) — official API docs
- [GLFW Documentation](https://www.glfw.org/docs/latest/) — window/input API
- [GLM Manual](https://glm.g-truc.net/0.9.9/api/index.html) — math library docs
- [Assimp Documentation](https://assimp-docs.readthedocs.io/) — model importer API
- [GLAD Generator](https://glad.dav1d.de/) — generate OpenGL loader

---

## License

This project is released for educational purposes.  
Third-party libraries (GLFW, GLM, Assimp, GLAD) retain their respective licenses.
