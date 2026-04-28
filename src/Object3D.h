#pragma once

// ============================================================
// Object3D.h — A single entity in the 3D scene.
//
// Each Object3D combines:
//   • A shared reference to a Model (geometry)
//   • A Material struct (Phong coefficients)
//   • A Transform (position / rotation / scale)
//
// The MODEL MATRIX is built from the transform each frame:
//   M = T * Rx * Ry * Rz * S
//   (translate, then rotate XYZ Euler, then scale)
//
// Multiple Object3Ds can share the same Model — the geometry
// lives on GPU once; only the uniforms differ per draw call.
// ============================================================

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "Model.h"
#include "Shader.h"

// ---- Material ---------------------------------------------
// Phong shading coefficients.  These map to uniforms in
// phong.frag with the prefix "material_".
//
//   ambient  (ka) — self-illumination; keeps objects visible
//                   even when not lit
//   diffuse  (kd) — main surface colour under direct light
//   specular (ks) — mirror-like highlight strength
//   shininess     — exponent controlling highlight size:
//                   high = tight/glossy, low = broad/matte
// -----------------------------------------------------------
struct Material {
    glm::vec3 ambient   = glm::vec3(0.1f);
    glm::vec3 diffuse   = glm::vec3(0.8f, 0.2f, 0.2f);
    glm::vec3 specular  = glm::vec3(0.5f);
    float     shininess = 32.f;
    glm::vec3 color     = glm::vec3(0.8f, 0.8f, 0.8f); // base albedo
};

// ---- Object3D ---------------------------------------------
class Object3D {
public:
    std::string            name;      // display name (shown in selection)
    std::shared_ptr<Model> model;     // shared geometry (can be null for primitives)
    Material               material;

    // Transform state — rebuilt into modelMatrix each call to getModelMatrix()
    glm::vec3 position = glm::vec3(0.f);
    glm::vec3 rotation = glm::vec3(0.f); // Euler angles in degrees (X, Y, Z)
    glm::vec3 scale    = glm::vec3(1.f);

    // True when this is the currently selected object (renderer adds a tint)
    bool isSelected = false;

    Object3D(const std::string& name, std::shared_ptr<Model> model);

    // ---- Matrix ----

    // Recompute and return the 4×4 model matrix.
    // Applies: Translate → Rotate-X → Rotate-Y → Rotate-Z → Scale
    glm::mat4 getModelMatrix() const;

    // ---- Draw ----

    // Upload all object-specific uniforms to 'shader' and draw.
    // The caller must have called shader.use() and set the
    // view/projection uniforms before calling this.
    void draw(Shader& shader) const;

    // ---- Transform helpers ----

    void translate(const glm::vec3& delta);          // add delta to position
    void rotate   (const glm::vec3& deltaDeg);       // add deltaDeg (XYZ) to rotation
    void scaleBy  (const glm::vec3& factors);        // multiply scale per-axis
    void scaleBy  (float uniformFactor);             // uniform scale

    // Reset transform to identity
    void resetTransform();
};
