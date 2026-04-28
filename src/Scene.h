#pragma once

// ============================================================
// Scene.h — Container for all Object3Ds and scene-level state.
//
// Responsibilities:
//   • Owns a list of Object3D (via unique_ptr for clear ownership)
//   • Tracks the currently-selected object (activeIndex)
//   • Provides TAB-cycling to switch selection
//   • Stores the point light configuration
//
// The Scene does NOT own the Renderer or Camera — those are
// managed by main.cpp and passed in at render time.
// ============================================================

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Object3D.h"

// ---- PointLight -------------------------------------------
// A single point light source following the Phong model.
//
// Attenuation equation:
//   att = 1.0 / (constant + linear*d + quadratic*d^2)
//
// Default values match a "room-sized" light (~20 units range).
// -----------------------------------------------------------
struct PointLight {
    glm::vec3 position  = glm::vec3(3.f, 5.f, 3.f);

    // Colour components of the light itself
    glm::vec3 ambient   = glm::vec3(0.15f);
    glm::vec3 diffuse   = glm::vec3(1.0f);
    glm::vec3 specular  = glm::vec3(1.0f);

    // Attenuation coefficients (Ogre3D standard values)
    float constant  = 1.0f;
    float linear    = 0.045f;
    float quadratic = 0.0075f;
};

// ---- Scene ------------------------------------------------
class Scene {
public:
    std::vector<std::unique_ptr<Object3D>> objects;
    PointLight light;

    int activeIndex = 0; // index into objects[] for the selected object

    // Add an object to the scene; takes ownership
    void addObject(std::unique_ptr<Object3D> obj);

    // Cycle selection forward (wraps from last → first)
    void selectNext();

    // Cycle selection backward (wraps from first → last)
    void selectPrev();

    // Return pointer to the active object, or nullptr if scene is empty
    Object3D*       getActive();
    const Object3D* getActive() const;

    int objectCount() const { return static_cast<int>(objects.size()); }

    // Print a one-line status string to stdout (for debug HUD)
    void printStatus() const;

private:
    // Keep isSelected flags consistent with activeIndex
    void syncSelectionFlags();
};
