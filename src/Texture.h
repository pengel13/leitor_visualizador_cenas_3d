#pragma once

#include <string>
#include <glad/glad.h>

// Carrega uma textura 2D a partir de um arquivo de imagem (PNG, JPG, BMP, etc.)
// usando stb_image. Mantém um cache por caminho para evitar recarregamentos.
// Retorna 0 se o arquivo não for encontrado (falha silenciosa — objeto fica com cor do material).
GLuint carregarTextura(const std::string& caminho);
