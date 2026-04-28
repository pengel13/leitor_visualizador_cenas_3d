Place your 3D model files here.

Supported formats (via Assimp):
  .obj  — Wavefront Object (recommended; triangulate in Blender before export)
  .ply  — Polygon File Format (Stanford)
  .fbx  — Autodesk FBX
  .gltf / .glb  — GL Transmission Format
  .dae  — COLLADA
  .3ds  — 3D Studio Max

Tips:
  - Export with triangulated faces from Blender (enable "Triangulate Faces" option)
  - Normals should be included in the file; Assimp can generate them if missing
  - UV coordinates are optional (the shader uses them for texture mapping)
  - Use MeshLab to clean up and reduce polygon count before loading

Usage:
  Run the viewer with your model file as an argument:
    SceneViewer3D.exe assets/models/your_model.obj

  Or drag-and-drop model files onto the executable.
