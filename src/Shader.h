#pragma once

#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
public:
    GLuint id = 0;

    Shader(const std::string& caminhoVert, const std::string& caminhoFrag);
    ~Shader();

    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;

    void usar()      const;
    void desativar() const;

    void definirInt   (const std::string& nome, int          valor) const;
    void definirFloat (const std::string& nome, float        valor) const;
    void definirBool  (const std::string& nome, bool         valor) const;
    void definirVec2  (const std::string& nome, const glm::vec2& v) const;
    void definirVec3  (const std::string& nome, const glm::vec3& v) const;
    void definirVec4  (const std::string& nome, const glm::vec4& v) const;
    void definirMat3  (const std::string& nome, const glm::mat3& m) const;
    void definirMat4  (const std::string& nome, const glm::mat4& m) const;

private:
    static GLuint      compilarEstagio(const std::string& src, GLenum tipo);
    static std::string lerArquivo(const std::string& caminho);
    static void        verificarErros(GLuint objeto, bool isPrograma);

    mutable std::unordered_map<std::string, GLint> cacheUniform;
    GLint obterLocalizacaoUniform(const std::string& nome) const;
};
