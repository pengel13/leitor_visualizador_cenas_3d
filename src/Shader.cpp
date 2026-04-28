#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>

// ============================================================
// readFile — Reads the entire contents of a text file into a
// std::string.  Throws if the file cannot be opened so that
// shader compilation errors are caught early with a useful
// message instead of a cryptic GL_COMPILE_STATUS failure.
// ============================================================
std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("[Shader] Cannot open file: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// ============================================================
// compileStage — Compile a single GLSL stage.
//
// 'type' is either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
// Returns the shader object handle; the caller must delete it
// with glDeleteShader after the program has been linked.
// ============================================================
GLuint Shader::compileStage(const std::string& src, GLenum type) {
    GLuint shader = glCreateShader(type);

    // glShaderSource takes an array of strings; we pass one.
    const char* srcPtr = src.c_str();
    glShaderSource(shader, 1, &srcPtr, nullptr);
    glCompileShader(shader);

    checkErrors(shader, false /* isProgram = false → check shader */);
    return shader;
}

// ============================================================
// checkErrors — Query GL compile / link status and print any
// info log to stderr.  Does nothing if the operation succeeded.
// ============================================================
void Shader::checkErrors(GLuint object, bool isProgram) {
    GLint  success = 0;
    char   infoLog[2048] = {};

    if (isProgram) {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "[Shader] Link error:\n" << infoLog << "\n";
        }
    } else {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "[Shader] Compile error:\n" << infoLog << "\n";
        }
    }
}

// ============================================================
// Constructor — Read, compile, link.
// Individual stage objects are deleted after linking since
// they are no longer needed once the program is created.
// ============================================================
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::cout << "[Shader] Loading: " << vertexPath
              << " + " << fragmentPath << "\n";

    std::string vertSrc = readFile(vertexPath);
    std::string fragSrc = readFile(fragmentPath);

    GLuint vert = compileStage(vertSrc, GL_VERTEX_SHADER);
    GLuint frag = compileStage(fragSrc, GL_FRAGMENT_SHADER);

    // Create the program and attach both compiled stages
    id = glCreateProgram();
    glAttachShader(id, vert);
    glAttachShader(id, frag);
    glLinkProgram(id);
    checkErrors(id, true /* isProgram */);

    // Free intermediate stage objects — the linked program is self-contained
    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() {
    if (id) {
        glDeleteProgram(id);
    }
}

void Shader::use()   const { glUseProgram(id); }
void Shader::unuse() const { glUseProgram(0);  }

// ============================================================
// getUniformLocation — Resolve uniform name to location index.
//
// glGetUniformLocation is a relatively expensive driver call.
// We cache the result in uniformCache so that each name is
// resolved at most once per shader lifetime.
//
// Returns -1 if the name does not correspond to an active
// uniform (the driver may silently remove unused uniforms).
// ============================================================
GLint Shader::getUniformLocation(const std::string& name) const {
    auto it = uniformCache.find(name);
    if (it != uniformCache.end()) {
        return it->second;
    }

    GLint loc = glGetUniformLocation(id, name.c_str());
    // loc == -1 means the uniform is not active (possibly optimised away)
    uniformCache[name] = loc;
    return loc;
}

// ---- Uniform setters ----------------------------------------
// Each setter resolves (or retrieves) the location, then calls
// the appropriate glUniform* to upload the value to GPU memory.
// The shader must be bound (use() called) before setting uniforms.
// -------------------------------------------------------------
void Shader::setInt  (const std::string& n, int   v) const {
    glUniform1i(getUniformLocation(n), v);
}

void Shader::setFloat(const std::string& n, float v) const {
    glUniform1f(getUniformLocation(n), v);
}

void Shader::setBool (const std::string& n, bool  v) const {
    glUniform1i(getUniformLocation(n), static_cast<int>(v));
}

void Shader::setVec2(const std::string& n, const glm::vec2& v) const {
    glUniform2fv(getUniformLocation(n), 1, glm::value_ptr(v));
}

void Shader::setVec3(const std::string& n, const glm::vec3& v) const {
    glUniform3fv(getUniformLocation(n), 1, glm::value_ptr(v));
}

void Shader::setVec4(const std::string& n, const glm::vec4& v) const {
    glUniform4fv(getUniformLocation(n), 1, glm::value_ptr(v));
}

void Shader::setMat3(const std::string& n, const glm::mat3& m) const {
    // GL_FALSE = do NOT transpose (GLM stores column-major, same as GL)
    glUniformMatrix3fv(getUniformLocation(n), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::setMat4(const std::string& n, const glm::mat4& m) const {
    glUniformMatrix4fv(getUniformLocation(n), 1, GL_FALSE, glm::value_ptr(m));
}
