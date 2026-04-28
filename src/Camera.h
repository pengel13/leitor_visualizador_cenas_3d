#pragma once

// ============================================================
// Camera.h — FPS-style first-person camera.
//
// Coordinate system: right-handed, Y-up (standard OpenGL).
//
// The camera is defined by:
//   • position   — world-space eye location
//   • yaw        — rotation around world Y axis (left/right look)
//   • pitch      — rotation around camera X axis (up/down look)
//
// From yaw and pitch we derive three orthonormal vectors:
//   • front — the direction the camera looks toward
//   • right — camera's local X axis
//   • up    — camera's local Y axis
//
// The VIEW matrix maps from world space to camera/eye space:
//   view = lookAt(position, position + front, up)
//
// The PROJECTION matrix maps from camera space to clip space.
// Two modes are supported: Perspective and Orthographic.
// ============================================================

#include <glm/glm.hpp>

// Which projection matrix to compute
enum class ProjectionMode { Perspective, Orthographic };

class Camera {
public:
    // ---- Position & orientation ----
    glm::vec3 position;   // world-space eye position
    glm::vec3 worldUp;    // world up direction (usually +Y)

    // Euler angles in degrees
    float yaw   = -90.f;  // -90 makes the camera look along -Z initially
    float pitch =   0.f;

    // ---- Derived direction vectors (updated by updateVectors) ----
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

    // ---- Movement parameters ----
    float moveSpeed        = 5.f;   // world units per second
    float mouseSensitivity = 0.08f; // degrees per pixel

    // ---- Projection parameters ----
    float fov         = 45.f;   // vertical field of view in degrees (perspective)
    float nearPlane   = 0.1f;
    float farPlane    = 500.f;
    float aspectRatio = 16.f / 9.f;
    float orthoScale  = 5.f;    // half-width in orthographic mode

    ProjectionMode projMode = ProjectionMode::Perspective;

    // ---- Constructor ----
    explicit Camera(glm::vec3 startPos  = glm::vec3(0.f, 2.f, 8.f),
                    float     aspect    = 16.f / 9.f);

    // ---- Matrix generators ----
    glm::mat4 getViewMatrix()       const;
    glm::mat4 getProjectionMatrix() const;

    // ---- Input handlers ----

    // Called each frame with WASD/Space/Ctrl flags
    enum Direction { FORWARD, BACKWARD, LEFT, RIGHT, UP_DIR, DOWN_DIR };
    void processKeyboard(Direction dir, float deltaTime);

    // Called from mouse-move callback (raw pixel deltas)
    // constrainPitch = true keeps pitch in [-89°, +89°]
    void processMouseMovement(float xOffset, float yOffset,
                              bool constrainPitch = true);

    // Called from scroll callback (affects fov in perspective mode)
    void processMouseScroll(float yOffset);

    // Toggle between perspective and orthographic
    void toggleProjection();

    // Recompute front/right/up from current yaw and pitch.
    // Must be called after any change to yaw or pitch.
    void updateVectors();
};
