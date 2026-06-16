#include "Texture.h"

#include <iostream>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static std::unordered_map<std::string, GLuint> s_cacheTexturas;

GLuint carregarTextura(const std::string& caminho) {
    auto it = s_cacheTexturas.find(caminho);
    if (it != s_cacheTexturas.end()) {
        return it->second;
    }

    // stb_image tem Y=0 em cima; OpenGL espera Y=0 embaixo
    stbi_set_flip_vertically_on_load(true);

    int largura, altura, canais;
    unsigned char* dados = stbi_load(caminho.c_str(), &largura, &altura, &canais, 0);

    if (!dados) {
        std::cerr << "[Textura] Nao foi possivel carregar: " << caminho << "\n";
        s_cacheTexturas[caminho] = 0;
        return 0;
    }

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    GLenum formato = (canais == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(formato),
                 largura, altura, 0,
                 formato, GL_UNSIGNED_BYTE, dados);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(dados);
    glBindTexture(GL_TEXTURE_2D, 0);

    s_cacheTexturas[caminho] = id;
    std::cout << "[Textura] Carregada: " << caminho
              << " (" << largura << "x" << altura << ", " << canais << " canais)\n";
    return id;
}
