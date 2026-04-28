#pragma once

#include "Mesh.h"

namespace Primitivos {
    Malha criarCubo(float tamanho = 1.0f);
    Malha criarEsfera(float raio = 0.5f, int setores = 36, int pilhas = 18);
    Malha criarPlano(float tamanho = 1.0f);
}
