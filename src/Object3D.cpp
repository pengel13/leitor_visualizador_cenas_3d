#include "Object3D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ============================================================
// Constructor
// ============================================================
Object3D::Object3D(const std::string& objectName, std::shared_ptr<Model> objectModel)
    : name(objectName)
    , model(std::move(objectModel))
{}

// ============================================================
// getModelMatrix — Build the 4×4 model transform from position,
// rotation (Euler XYZ, degrees) and scale.
//
// Matrix composition order:
//   M = T * Rx * Ry * Rz * S
//
// This means scale is applied first, then rotations in XYZ
// order, then translation — the standard "SRT" decomposition.
//
// The normal matrix used in the shader must be:
//   normalMatrix = mat3(transpose(inverse(model)))
// It handles non-uniform scaling so normals stay perpendicular
// to the surface even after stretching.
// ============================================================
glm::mat4 Object3D::getModelMatrix() const {
    glm::mat4 m = glm::mat4(1.f);

    // Translation
    m = glm::translate(m, position);

    // Rotation: X first, then Y, then Z (Euler XYZ extrinsic)
    m = glm::rotate(m, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
    m = glm::rotate(m, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
    m = glm::rotate(m, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));

    // Scale
    m = glm::scale(m, scale);

    return m;
}

// ============================================================
// draw — Upload per-object uniforms and issue draw call.
//
// Uniforms set here:
//   model        — 4×4 model matrix
//   normalMatrix — 3×3 for correct normal transformation
//   material_*   — Phong coefficients (handled by Renderer)
//   isSelected   — highlight tint flag
//
// Note: view, projection, light, viewPos are set by Renderer
// because they are shared across all objects in the frame.
// ============================================================
void Object3D::draw(Shader& shader) const {
    glm::mat4 modelMat = getModelMatrix();

    // Normal matrix: transpose of inverse of the 3×3 upper-left of model.
    // This correctly transforms normals when model has non-uniform scale.
    glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(modelMat)));

    shader.setMat4("model",        modelMat);
    shader.setMat3("normalMatrix", normalMat);
    shader.setBool("isSelected",   isSelected);

    if (model) {
        model->draw();
    }
}

// ============================================================
// Transform helpers
// ============================================================
void Object3D::translate(const glm::vec3& delta) {
    position += delta;
}

void Object3D::rotate(const glm::vec3& deltaDeg) {
    rotation += deltaDeg;
}

void Object3D::scaleBy(const glm::vec3& factors) {
    scale *= factors;
}

void Object3D::scaleBy(float uniformFactor) {
    scale *= uniformFactor;
}

void Object3D::resetTransform() {
    position = glm::vec3(0.f);
    rotation = glm::vec3(0.f);
    scale    = glm::vec3(1.f);
}
