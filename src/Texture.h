#pragma once

#include <string>
#include <glad/glad.h>

// Carrega textura 2D via stb_image. Cache por caminho. Retorna 0 se falhar.
GLuint carregarTextura(const std::string& caminho);
