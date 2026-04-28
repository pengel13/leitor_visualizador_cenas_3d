#include "Scene.h"

#include <iostream>
#include <algorithm>

// ============================================================
// addObject — Transfer ownership of an Object3D to the scene.
// The first object added automatically becomes the selection.
// ============================================================
void Scene::addObject(std::unique_ptr<Object3D> obj) {
    objects.push_back(std::move(obj));
    syncSelectionFlags();
}

// ============================================================
// selectNext — Move selection forward by one, wrapping around.
// If the scene is empty nothing changes.
// ============================================================
void Scene::selectNext() {
    if (objects.empty()) return;
    activeIndex = (activeIndex + 1) % static_cast<int>(objects.size());
    syncSelectionFlags();
    std::cout << "[Scene] Selected: " << objects[activeIndex]->name
              << " (" << activeIndex + 1 << "/" << objects.size() << ")\n";
}

// ============================================================
// selectPrev — Move selection backward by one, wrapping around.
// ============================================================
void Scene::selectPrev() {
    if (objects.empty()) return;
    activeIndex = (activeIndex - 1 + static_cast<int>(objects.size()))
                  % static_cast<int>(objects.size());
    syncSelectionFlags();
    std::cout << "[Scene] Selected: " << objects[activeIndex]->name
              << " (" << activeIndex + 1 << "/" << objects.size() << ")\n";
}

// ============================================================
// getActive — Return a pointer to the selected object,
// or nullptr when the scene has no objects.
// ============================================================
Object3D* Scene::getActive() {
    if (objects.empty()) return nullptr;
    return objects[activeIndex].get();
}

const Object3D* Scene::getActive() const {
    if (objects.empty()) return nullptr;
    return objects[activeIndex].get();
}

// ============================================================
// printStatus — Print a one-line scene summary (for debug HUD).
// ============================================================
void Scene::printStatus() const {
    if (objects.empty()) {
        std::cout << "[Scene] Empty\n";
        return;
    }
    const Object3D* a = getActive();
    std::cout << "[Scene] Objects: " << objects.size()
              << "  Active: " << a->name
              << "  Pos: (" << a->position.x << ", "
                            << a->position.y << ", "
                            << a->position.z << ")\n";
}

// ============================================================
// syncSelectionFlags — Keep isSelected consistent with activeIndex.
// Only one object should have isSelected == true at a time.
// ============================================================
void Scene::syncSelectionFlags() {
    for (int i = 0; i < static_cast<int>(objects.size()); ++i) {
        objects[i]->isSelected = (i == activeIndex);
    }
}
