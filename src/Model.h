#pragma once

// ============================================================
// Model.h — Loads a 3D file via Assimp into a list of Meshes.
//
// Assimp (Open Asset Import Library) handles the heavy lifting
// of parsing dozens of 3D formats (.obj, .ply, .fbx, .gltf…)
// and gives us a normalised scene graph (aiScene).
//
// This class walks that graph, converts each aiMesh to our Mesh
// type, and stores them all in a flat vector.
//
// Supported formats depend on the Assimp build, but at minimum:
//   .obj, .ply, .fbx, .dae, .3ds, .gltf/.glb
//
// Usage:
//   Model m("assets/models/cube.obj");
//   m.draw();   // draws all sub-meshes
// ============================================================

#include <string>
#include <vector>
#include "Mesh.h"

// Forward-declare Assimp types to avoid polluting every
// translation unit that includes this header.
struct aiScene;
struct aiNode;
struct aiMesh;

class Model {
public:
    std::vector<Mesh> meshes;    // flat collection of all sub-meshes
    std::string       filePath;  // original path (for error messages)
    std::string       directory; // directory containing the file

    // Default constructor — creates an empty model.
    // Meshes can be pushed into the meshes vector manually.
    // Used by Primitives helpers to wrap procedural geometry.
    Model() = default;

    // Load a model from disk.
    // Import flags applied internally:
    //   aiProcess_Triangulate        — all faces become triangles
    //   aiProcess_GenSmoothNormals   — generate per-vertex normals if absent
    //   aiProcess_FlipUVs            — OpenGL expects UV origin at bottom-left
    //   aiProcess_JoinIdenticalVertices — merge duplicate vertices
    //   aiProcess_OptimizeMeshes     — reduce number of draw calls
    //
    // Throws std::runtime_error if the file cannot be opened or parsed.
    explicit Model(const std::string& path);

    // Draw all sub-meshes sequentially (no shader binding — caller's job).
    void draw() const;

private:
    // Recursively walk assimp's node tree and process each aiMesh
    void processNode(const aiNode* node, const aiScene* scene);

    // Convert a single assimp mesh into our GPU-ready Mesh format
    Mesh processMesh(const aiMesh* mesh, const aiScene* scene);
};
