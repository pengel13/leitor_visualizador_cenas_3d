#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>

std::string Shader::lerArquivo(const std::string& caminho) {
    std::ifstream arquivo(caminho);
    if (!arquivo.is_open()) {
        throw std::runtime_error("[Shader] Nao foi possivel abrir: " + caminho);
    }
    std::ostringstream ss;
    ss << arquivo.rdbuf();
    return ss.str();
}

GLuint Shader::compilarEstagio(const std::string& src, GLenum tipo) {
    GLuint shader = glCreateShader(tipo);
    const char* srcPtr = src.c_str();
    glShaderSource(shader, 1, &srcPtr, nullptr);
    glCompileShader(shader);
    verificarErros(shader, false);
    return shader;
}

void Shader::verificarErros(GLuint objeto, bool isPrograma) {
    GLint success = 0;
    char  infoLog[2048] = {};

    if (isPrograma) {
        glGetProgramiv(objeto, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(objeto, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "[Shader] Erro de link:\n" << infoLog << "\n";
        }
    } else {
        glGetShaderiv(objeto, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(objeto, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "[Shader] Erro de compilacao:\n" << infoLog << "\n";
        }
    }
}

Shader::Shader(const std::string& caminhoVert, const std::string& caminhoFrag) {
    std::cout << "[Shader] Carregando: " << caminhoVert
              << " + " << caminhoFrag << "\n";

    std::string srcVert = lerArquivo(caminhoVert);
    std::string srcFrag = lerArquivo(caminhoFrag);

    GLuint vert = compilarEstagio(srcVert, GL_VERTEX_SHADER);
    GLuint frag = compilarEstagio(srcFrag, GL_FRAGMENT_SHADER);

    id = glCreateProgram();
    glAttachShader(id, vert);
    glAttachShader(id, frag);
    glLinkProgram(id);
    verificarErros(id, true);

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() {
    if (id) {
        glDeleteProgram(id);
    }
}

void Shader::usar()      const { glUseProgram(id); }
void Shader::desativar() const { glUseProgram(0);  }

GLint Shader::obterLocalizacaoUniform(const std::string& nome) const {
    auto it = cacheUniform.find(nome);
    if (it != cacheUniform.end()) {
        return it->second;
    }
    GLint loc = glGetUniformLocation(id, nome.c_str());
    cacheUniform[nome] = loc;
    return loc;
}

void Shader::definirInt  (const std::string& n, int   v) const {
    glUniform1i(obterLocalizacaoUniform(n), v);
}

void Shader::definirFloat(const std::string& n, float v) const {
    glUniform1f(obterLocalizacaoUniform(n), v);
}

void Shader::definirBool (const std::string& n, bool  v) const {
    glUniform1i(obterLocalizacaoUniform(n), static_cast<int>(v));
}

void Shader::definirVec2(const std::string& n, const glm::vec2& v) const {
    glUniform2fv(obterLocalizacaoUniform(n), 1, glm::value_ptr(v));
}

void Shader::definirVec3(const std::string& n, const glm::vec3& v) const {
    glUniform3fv(obterLocalizacaoUniform(n), 1, glm::value_ptr(v));
}

void Shader::definirVec4(const std::string& n, const glm::vec4& v) const {
    glUniform4fv(obterLocalizacaoUniform(n), 1, glm::value_ptr(v));
}

void Shader::definirMat3(const std::string& n, const glm::mat3& m) const {
    // GL_FALSE: nao transpor — GLM usa coluna-maior, igual ao OpenGL
    glUniformMatrix3fv(obterLocalizacaoUniform(n), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::definirMat4(const std::string& n, const glm::mat4& m) const {
    glUniformMatrix4fv(obterLocalizacaoUniform(n), 1, GL_FALSE, glm::value_ptr(m));
}
