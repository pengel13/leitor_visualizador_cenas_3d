#include "Camera.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// ============================================================
// Constructor — set defaults and compute initial direction vectors.
// ============================================================
Camera::Camera(glm::vec3 startPos, float aspect)
    : position(startPos)
    , worldUp(0.f, 1.f, 0.f)
    , aspectRatio(aspect)
{
    updateVectors();
}

// ============================================================
// updateVectors — Derive front, right, up from current yaw/pitch.
//
// This is called whenever yaw or pitch changes.
//
// Mathematical derivation (right-handed, Y-up):
//   front.x = cos(yaw) * cos(pitch)
//   front.y = sin(pitch)
//   front.z = sin(yaw) * cos(pitch)
//
// Then we orthogonalise:
//   right = normalize(cross(front, worldUp))
//   up    = normalize(cross(right, front))
// ============================================================
void Camera::updateVectors() {
    float yawR   = glm::radians(yaw);
    float pitchR = glm::radians(pitch);

    front.x = std::cos(yawR) * std::cos(pitchR);
    front.y = std::sin(pitchR);
    front.z = std::sin(yawR) * std::cos(pitchR);
    front   = glm::normalize(front);

    // Gram-Schmidt: derive right and up from front and world-up
    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}

// ============================================================
// getViewMatrix — Compute the view (world→eye) transform.
//
// glm::lookAt takes:
//   eye    = camera position
//   center = point the camera looks at (eye + front)
//   up     = camera's local up direction
// ============================================================
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

// ============================================================
// getProjectionMatrix — Compute the projection (eye→clip) matrix.
//
// Perspective: standard 4×4 perspective frustum.
// Orthographic: an orthographic frustum symmetric around origin.
//   The orthoScale value controls how many world units fit
//   from the centre to the edge of the screen.
// ============================================================
glm::mat4 Camera::getProjectionMatrix() const {
    if (projMode == ProjectionMode::Perspective) {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }

    // Ortho: compute half-extents from aspect ratio and ortho scale
    float hw = orthoScale * aspectRatio; // half-width
    float hh = orthoScale;               // half-height
    return glm::ortho(-hw, hw, -hh, hh, nearPlane, farPlane);
}

// ============================================================
// processKeyboard — Translate the camera position.
//
// deltaTime (seconds) ensures frame-rate-independent movement.
// Speed is in world units per second.
// ============================================================
void Camera::processKeyboard(Direction dir, float deltaTime) {
    float velocity = moveSpeed * deltaTime;

    switch (dir) {
        case FORWARD:  position += front   * velocity; break;
        case BACKWARD: position -= front   * velocity; break;
        case LEFT:     position -= right   * velocity; break;
        case RIGHT:    position += right   * velocity; break;
        case UP_DIR:   position += worldUp * velocity; break;
        case DOWN_DIR: position -= worldUp * velocity; break;
    }
}

// ============================================================
// processMouseMovement — Update yaw and pitch from mouse deltas.
//
// xOffset/yOffset are raw pixel deltas (positive = right/up).
// mouseSensitivity converts pixels to degrees.
//
// constrainPitch clamps pitch to [-89°, +89°] to prevent the
// camera from flipping when looking straight up or down.
// ============================================================
void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw   += xOffset;
    pitch += yOffset;

    if (constrainPitch) {
        pitch = std::clamp(pitch, -89.f, 89.f);
    }

    updateVectors();
}

// ============================================================
// processMouseScroll — Zoom by adjusting field of view.
// Clamped to [1°, 90°] so the view never inverts or collapses.
// ============================================================
void Camera::processMouseScroll(float yOffset) {
    fov -= yOffset * 2.f;
    fov  = std::clamp(fov, 1.f, 90.f);
}

// ============================================================
// toggleProjection — Switch between perspective and ortho.
// ============================================================
void Camera::toggleProjection() {
    projMode = (projMode == ProjectionMode::Perspective)
               ? ProjectionMode::Orthographic
               : ProjectionMode::Perspective;
}
