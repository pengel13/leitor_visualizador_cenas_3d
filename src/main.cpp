// ============================================================
// main.cpp — 3D Scene Viewer entry point.
//
// Initialization order:
//   1. GLFW init + window creation
//   2. GLAD — loads OpenGL function pointers
//   3. OpenGL state (depth test, viewport)
//   4. Renderer::init() — compiles shaders, builds grid/axes GPU data
//   5. Scene setup — add objects (from .obj files or procedural)
//   6. Main loop:
//        a. Poll events / process continuous key state
//        b. renderer.render(scene, camera)
//        c. Swap buffers
//   7. Cleanup
//
// Key bindings:
//   WASD            — move camera
//   Mouse           — look around (FPS style)
//   Space / C       — move camera up / down
//   TAB             — select next object
//   Shift+TAB       — select previous object
//   Arrow keys      — translate selected object
//   Page Up/Down    — translate selected object along Y
//   Hold R + Arrows — rotate selected object
//   Hold R + ,/.    — rotate selected object around Z
//   = / +           — scale up selected object
//   - / _           — scale down selected object
//   P               — toggle perspective / orthographic
//   F               — toggle wireframe overlay
//   G               — toggle ground grid
//   H               — toggle XYZ axes
//   L               — print scene status to stdout
//   Escape          — release mouse cursor / exit
// ============================================================

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

// GLAD must be included before GLFW; it replaces <GL/gl.h>
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

// ============================================================
// Window configuration constants
// ============================================================
static constexpr int   WINDOW_W     = 1280;
static constexpr int   WINDOW_H     = 720;
static constexpr char  WINDOW_TITLE[] = "3D Scene Viewer — Modern OpenGL";

// ============================================================
// Global state accessed by GLFW callbacks.
// Using globals here is common practice for GLFW callbacks;
// a production app would use glfwSetWindowUserPointer instead.
// ============================================================
static Camera*   g_camera   = nullptr;
static Scene*    g_scene    = nullptr;
static Renderer* g_renderer = nullptr;

// Mouse tracking
static bool  g_firstMouse   = true;
static float g_lastX        = WINDOW_W / 2.f;
static float g_lastY        = WINDOW_H / 2.f;
static bool  g_mouseCaptured = true;  // start in capture mode

// Per-frame key state (polled each frame for smooth movement)
static bool g_keys[GLFW_KEY_LAST + 1] = {};

// ============================================================
// GLFW Callbacks
// ============================================================

// framebufferSizeCallback — called whenever the window is resized.
// We update the OpenGL viewport AND the camera aspect ratio so
// the projection matrix stays correct.
static void framebufferSizeCallback(GLFWwindow* /*win*/, int w, int h) {
    if (h == 0) return; // guard against division by zero
    glViewport(0, 0, w, h);
    if (g_camera) {
        g_camera->aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    }
}

// mouseMoveCallback — called on every mouse movement.
// Only rotates the camera when the cursor is captured.
static void mouseMoveCallback(GLFWwindow* /*win*/, double xpos, double ypos) {
    if (!g_camera || !g_mouseCaptured) return;

    float fx = static_cast<float>(xpos);
    float fy = static_cast<float>(ypos);

    if (g_firstMouse) {
        g_lastX = fx;
        g_lastY = fy;
        g_firstMouse = false;
    }

    float xOffset =  (fx - g_lastX); // right is +yaw
    float yOffset =  (g_lastY - fy); // up is +pitch (screen Y is flipped)

    g_lastX = fx;
    g_lastY = fy;

    g_camera->processMouseMovement(xOffset, yOffset);
}

// scrollCallback — zoom via field-of-view change
static void scrollCallback(GLFWwindow* /*win*/, double /*x*/, double yOffset) {
    if (g_camera) {
        g_camera->processMouseScroll(static_cast<float>(yOffset));
    }
}

// keyCallback — handles single-press actions (not held movement).
// Continuous movement is handled by polling g_keys[] in the loop.
static void keyCallback(GLFWwindow* win, int key, int /*scan*/, int action, int mods) {
    // Track raw key state for polling
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        if (action == GLFW_PRESS)   g_keys[key] = true;
        if (action == GLFW_RELEASE) g_keys[key] = false;
    }

    if (action != GLFW_PRESS) return; // only handle single-press events below

    switch (key) {

        // ---- Mouse capture toggle ----
        case GLFW_KEY_ESCAPE:
            if (g_mouseCaptured) {
                // Release cursor
                glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                g_mouseCaptured = false;
                g_firstMouse    = true;
            } else {
                // Exit application
                glfwSetWindowShouldClose(win, GLFW_TRUE);
            }
            break;

        // Re-capture cursor on left click (handled in mouse button callback)

        // ---- Object selection (TAB / Shift+TAB) ----
        case GLFW_KEY_TAB:
            if (g_scene) {
                if (mods & GLFW_MOD_SHIFT) g_scene->selectPrev();
                else                       g_scene->selectNext();
            }
            break;

        // ---- Rendering toggles ----
        case GLFW_KEY_P:
            if (g_camera) g_camera->toggleProjection();
            std::cout << "[Camera] Projection: "
                      << (g_camera->projMode == ProjectionMode::Perspective
                          ? "Perspective" : "Orthographic") << "\n";
            break;

        case GLFW_KEY_F:
            if (g_renderer) {
                g_renderer->wireframeEnabled = !g_renderer->wireframeEnabled;
                std::cout << "[Renderer] Wireframe: "
                          << (g_renderer->wireframeEnabled ? "ON" : "OFF") << "\n";
            }
            break;

        case GLFW_KEY_G:
            if (g_renderer) {
                g_renderer->showGrid = !g_renderer->showGrid;
                std::cout << "[Renderer] Grid: "
                          << (g_renderer->showGrid ? "ON" : "OFF") << "\n";
            }
            break;

        case GLFW_KEY_H:
            if (g_renderer) {
                g_renderer->showAxes = !g_renderer->showAxes;
                std::cout << "[Renderer] Axes: "
                          << (g_renderer->showAxes ? "ON" : "OFF") << "\n";
            }
            break;

        // ---- Scene status dump ----
        case GLFW_KEY_L:
            if (g_scene) g_scene->printStatus();
            break;

        // ---- Reset selected object transform ----
        case GLFW_KEY_BACKSPACE:
            if (g_scene) {
                Object3D* active = g_scene->getActive();
                if (active) {
                    active->resetTransform();
                    std::cout << "[Object3D] Reset transform: " << active->name << "\n";
                }
            }
            break;

        default: break;
    }
}

// mouseButtonCallback — recapture cursor on left click
static void mouseButtonCallback(GLFWwindow* win, int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !g_mouseCaptured) {
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        g_mouseCaptured = true;
        g_firstMouse    = true; // prevent jump on recapture
    }
}

// ============================================================
// processInput — Called every frame to handle held keys.
//
// Camera movement and object transforms are smoothed by
// deltaTime (seconds since last frame) for frame-rate independence.
// ============================================================
static void processInput(float deltaTime) {
    if (!g_camera || !g_scene) return;

    // ---- Camera movement (WASD + Space/C) ----
    if (g_keys[GLFW_KEY_W])            g_camera->processKeyboard(Camera::FORWARD,  deltaTime);
    if (g_keys[GLFW_KEY_S])            g_camera->processKeyboard(Camera::BACKWARD, deltaTime);
    if (g_keys[GLFW_KEY_A])            g_camera->processKeyboard(Camera::LEFT,     deltaTime);
    if (g_keys[GLFW_KEY_D])            g_camera->processKeyboard(Camera::RIGHT,    deltaTime);
    if (g_keys[GLFW_KEY_SPACE])        g_camera->processKeyboard(Camera::UP_DIR,   deltaTime);
    if (g_keys[GLFW_KEY_C])            g_camera->processKeyboard(Camera::DOWN_DIR, deltaTime);

    // ---- Object transforms ----
    Object3D* obj = g_scene->getActive();
    if (!obj) return;

    const float tSpeed = 3.f * deltaTime;   // translation speed (units/sec)
    const float rSpeed = 90.f * deltaTime;  // rotation speed    (degrees/sec)
    const float sUp    = 1.f + 0.8f * deltaTime; // scale-up factor
    const float sDown  = 1.f / sUp;              // scale-down factor

    bool rHeld = g_keys[GLFW_KEY_R]; // hold R for rotation mode

    if (!rHeld) {
        // Translation mode (Arrow keys + Page Up/Down)
        if (g_keys[GLFW_KEY_UP])        obj->translate({ 0.f,  0.f, -tSpeed});
        if (g_keys[GLFW_KEY_DOWN])      obj->translate({ 0.f,  0.f,  tSpeed});
        if (g_keys[GLFW_KEY_LEFT])      obj->translate({-tSpeed, 0.f,  0.f});
        if (g_keys[GLFW_KEY_RIGHT])     obj->translate({ tSpeed, 0.f,  0.f});
        if (g_keys[GLFW_KEY_PAGE_UP])   obj->translate({ 0.f,  tSpeed, 0.f});
        if (g_keys[GLFW_KEY_PAGE_DOWN]) obj->translate({ 0.f, -tSpeed, 0.f});
    } else {
        // Rotation mode (hold R, then Arrow keys / , . for Z)
        if (g_keys[GLFW_KEY_UP])        obj->rotate({-rSpeed, 0.f,   0.f});
        if (g_keys[GLFW_KEY_DOWN])      obj->rotate({ rSpeed, 0.f,   0.f});
        if (g_keys[GLFW_KEY_LEFT])      obj->rotate({ 0.f,  -rSpeed, 0.f});
        if (g_keys[GLFW_KEY_RIGHT])     obj->rotate({ 0.f,   rSpeed, 0.f});
        if (g_keys[GLFW_KEY_COMMA])     obj->rotate({ 0.f,   0.f, -rSpeed});
        if (g_keys[GLFW_KEY_PERIOD])    obj->rotate({ 0.f,   0.f,  rSpeed});
    }

    // Scale (= / +  to grow,  -  to shrink)
    if (g_keys[GLFW_KEY_EQUAL] || g_keys[GLFW_KEY_KP_ADD])      obj->scaleBy(sUp);
    if (g_keys[GLFW_KEY_MINUS] || g_keys[GLFW_KEY_KP_SUBTRACT]) obj->scaleBy(sDown);
}

// ============================================================
// createDemoScene — Populate the scene with built-in primitives
// and attempt to load any .obj files passed as command-line args.
//
// The demo always has at least a cube and a sphere so the viewer
// works even with no external model files.
// ============================================================
static void createDemoScene(Scene& scene, const std::vector<std::string>& modelPaths) {
    // ---- Built-in primitives ----
    // Helper: wrap a Mesh into a Model without loading from disk
    auto makePrimitiveModel = [](Mesh&& mesh) -> std::shared_ptr<Model> {
        auto m = std::make_shared<Model>();
        m->meshes.push_back(std::move(mesh));
        return m;
    };

    // Cube — red, left side
    {
        auto obj = std::make_unique<Object3D>(
            "Cube",
            makePrimitiveModel(Primitives::createCube(1.f))
        );
        obj->position     = glm::vec3(-2.f, 0.5f, 0.f);
        obj->material.color    = glm::vec3(0.8f, 0.2f, 0.2f); // red
        obj->material.diffuse  = glm::vec3(0.9f, 0.3f, 0.3f);
        obj->material.specular = glm::vec3(0.6f);
        obj->material.shininess = 64.f;
        scene.addObject(std::move(obj));
    }

    // Sphere — blue-green, right side
    {
        auto obj = std::make_unique<Object3D>(
            "Sphere",
            makePrimitiveModel(Primitives::createSphere(0.8f, 36, 24))
        );
        obj->position     = glm::vec3(2.f, 0.8f, 0.f);
        obj->material.color    = glm::vec3(0.2f, 0.5f, 0.9f); // blue
        obj->material.diffuse  = glm::vec3(0.3f, 0.5f, 0.9f);
        obj->material.specular = glm::vec3(0.8f);
        obj->material.shininess = 128.f;
        scene.addObject(std::move(obj));
    }

    // Ground plane — dark grey
    {
        auto obj = std::make_unique<Object3D>(
            "Ground",
            makePrimitiveModel(Primitives::createPlane(8.f))
        );
        obj->position = glm::vec3(0.f, 0.f, 0.f);
        obj->material.color    = glm::vec3(0.3f, 0.32f, 0.3f);
        obj->material.diffuse  = glm::vec3(0.3f);
        obj->material.specular = glm::vec3(0.05f);
        obj->material.shininess = 8.f;
        scene.addObject(std::move(obj));
    }

    // ---- Load external model files from command-line ----
    for (const auto& path : modelPaths) {
        try {
            auto model = std::make_shared<Model>(path);
            auto obj   = std::make_unique<Object3D>(path, model);
            // Auto-space objects along X axis
            float offset = static_cast<float>(scene.objectCount()) * 3.f - 3.f;
            obj->position = glm::vec3(offset, 0.f, -4.f);
            scene.addObject(std::move(obj));
        } catch (const std::exception& e) {
            std::cerr << "[Main] Failed to load model '" << path << "': "
                      << e.what() << "\n";
        }
    }

    // Light position
    scene.light.position = glm::vec3(4.f, 6.f, 4.f);
}

// ============================================================
// printUsage — Show startup information
// ============================================================
static void printUsage(const char* exeName) {
    std::cout
        << "┌─────────────────────────────────────────────┐\n"
        << "│         3D Scene Viewer — Modern OpenGL      │\n"
        << "└─────────────────────────────────────────────┘\n"
        << "Usage: " << exeName << " [model.obj] [model2.ply] ...\n\n"
        << "Controls:\n"
        << "  WASD / Space / C  — move camera\n"
        << "  Mouse             — look around\n"
        << "  TAB / Shift+TAB   — select next/prev object\n"
        << "  Arrow keys        — translate selected object\n"
        << "  Page Up/Down      — translate selected object up/down\n"
        << "  R + Arrows        — rotate selected object\n"
        << "  R + , / .         — rotate around Z axis\n"
        << "  = / -             — scale up / down\n"
        << "  P                 — toggle perspective/orthographic\n"
        << "  F                 — toggle wireframe overlay\n"
        << "  G                 — toggle ground grid\n"
        << "  H                 — toggle XYZ axes\n"
        << "  L                 — print scene status\n"
        << "  Backspace         — reset selected object transform\n"
        << "  Escape            — release mouse / exit\n\n";
}

// ============================================================
// main
// ============================================================
int main(int argc, char** argv) {
    // Collect optional model file paths from command-line args
    std::vector<std::string> modelPaths;
    for (int i = 1; i < argc; ++i) {
        modelPaths.emplace_back(argv[i]);
    }

    printUsage(argv[0]);

    // ---- 1. Initialise GLFW ----
    if (!glfwInit()) {
        std::cerr << "[GLFW] Initialisation failed.\n";
        return 1;
    }

    // Request OpenGL 4.6 Core Profile.
    // Core profile removes all deprecated "fixed pipeline" functions.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Uncomment for macOS compatibility:
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Enable multisampling (4x MSAA) for smoother edges
    glfwWindowHint(GLFW_SAMPLES, 4);

    // ---- 2. Create window and OpenGL context ----
    GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, WINDOW_TITLE,
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "[GLFW] Window creation failed.\n";
        glfwTerminate();
        return 1;
    }

    // Make this window's context current on the calling thread.
    // All subsequent OpenGL calls apply to this context.
    glfwMakeContextCurrent(window);

    // Enable VSync (0 = off, 1 = sync to monitor refresh)
    glfwSwapInterval(1);

    // ---- 3. Initialise GLAD ----
    // GLAD loads all OpenGL function pointers via GLFW's proc-address function.
    // This MUST happen after glfwMakeContextCurrent and BEFORE any gl* calls.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "[GLAD] Failed to load OpenGL function pointers.\n";
        glfwTerminate();
        return 1;
    }

    std::cout << "[OpenGL] Version: " << glGetString(GL_VERSION) << "\n"
              << "[OpenGL] Renderer: " << glGetString(GL_RENDERER) << "\n\n";

    // Enable 4x MSAA (must match the hint above)
    glEnable(GL_MULTISAMPLE);

    // ---- 4. Set up camera ----
    Camera camera(glm::vec3(0.f, 2.f, 8.f),
                  static_cast<float>(WINDOW_W) / static_cast<float>(WINDOW_H));
    g_camera = &camera;

    // ---- 5. Register callbacks ----
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback      (window, mouseMoveCallback);
    glfwSetScrollCallback         (window, scrollCallback);
    glfwSetKeyCallback            (window, keyCallback);
    glfwSetMouseButtonCallback    (window, mouseButtonCallback);

    // Capture the mouse cursor (hides it and locks it to the window)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ---- 6. Build scene ----
    Scene    scene;
    g_scene  = &scene;

    Renderer renderer;
    g_renderer = &renderer;

    // init() compiles shaders and builds GPU buffers — must be after GLAD
    renderer.init();

    // Populate the scene with primitives + any command-line models
    createDemoScene(scene, modelPaths);

    std::cout << "[Scene] " << scene.objectCount() << " objects loaded.\n"
              << "[Scene] Active: " << (scene.getActive() ? scene.getActive()->name : "none") << "\n\n";

    // ---- 7. Main render loop ----
    float lastTime  = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
        // Delta time — time since last frame in seconds
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime   = currentTime - lastTime;
        lastTime          = currentTime;

        // Clamp deltaTime to avoid huge jumps after alt-tab / breakpoints
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        // Process held-key input (camera movement + object transforms)
        processInput(deltaTime);

        // Draw the scene
        renderer.render(scene, camera);

        // Present the rendered frame
        glfwSwapBuffers(window);

        // Poll OS events (keyboard, mouse, window resize, etc.)
        glfwPollEvents();
    }

    // ---- 8. Cleanup ----
    // Objects with destructors (unique_ptr in Scene) handle GPU cleanup.
    // Renderer destructor deletes VAOs/VBOs.
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "[Main] Goodbye.\n";
    return 0;
}
