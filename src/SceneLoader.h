#pragma once

#include <string>
#include "Scene.h"
#include "Camera.h"

// Carrega uma cena completa a partir de um arquivo JSON.
// Preenche a Cena com os objetos, a iluminacao e configura a Camera
// conforme as definicoes no arquivo.
//
// Formato esperado do JSON (veja scene.json para exemplo completo):
//   camera: { posicao, guinada, arfagem, campoVisao, planoProximo, planoDistante }
//   luz:    { posicao, ambiente, difuso, especular, constante, linear, quadratico }
//   objetos: array de {
//     nome, arquivo | primitivo, posicao, rotacao, escala,
//     material (opcional), animacao (opcional)
//   }
//
// Objetos com "arquivo" inexistente sao ignorados com aviso no stderr.
// Objetos com "primitivo" suportado: "cubo", "esfera", "plano".
void carregarCena(const std::string& caminho, Cena& cena, Camera& camera);
