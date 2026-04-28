#pragma once

// ============================================================
// Shader.h — Wraps an OpenGL GLSL program (vertex + fragment).
//
// Responsibilities:
//   • Read shader source files from disk
//   • Compile each stage (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
//   • Link the stages into an OpenGL program object
//   • Provide typed uniform setters that cache location lookups
//
// Usage:
//   Shader sh("shaders/phong.vert", "shaders/phong.frag");
//   sh.use();
//   sh.setMat4("model", modelMatrix);
//   sh.setVec3("lightPos", lightPos);
// ============================================================

#include <string>
#include <unordered_map>

// GLAD must be included before any OpenGL header
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
public:
    // OpenGL program object handle — public so callers can use it directly
    GLuint id = 0;

    // Read GLSL sources from disk, compile and link.
    // Throws std::runtime_error if a file cannot be opened.
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    ~Shader();

    // Non-copyable; move is fine
    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;

    // Bind / unbind this shader program
    void use()   const;
    void unuse() const;

    // ---- Uniform setters ----
    // Each call resolves the uniform location (cached after first call)
    // and uploads the value to the currently-bound program.
    void setInt   (const std::string& name, int          value) const;
    void setFloat (const std::string& name, float        value) const;
    void setBool  (const std::string& name, bool         value) const;
    void setVec2  (const std::string& name, const glm::vec2& v) const;
    void setVec3  (const std::string& name, const glm::vec3& v) const;
    void setVec4  (const std::string& name, const glm::vec4& v) const;
    void setMat3  (const std::string& name, const glm::mat3& m) const;
    void setMat4  (const std::string& name, const glm::mat4& m) const;

private:
    // Compile a single GLSL stage; returns shader object handle
    static GLuint compileStage(const std::string& src, GLenum type);

    // Read an entire file into a std::string
    static std::string readFile(const std::string& path);

    // Print compile or link errors to stderr
    static void checkErrors(GLuint object, bool isProgram);

    // Uniform location cache — avoids calling glGetUniformLocation every frame
    mutable std::unordered_map<std::string, GLint> uniformCache;

    // Resolve (and cache) a uniform location by name
    GLint getUniformLocation(const std::string& name) const;
};
