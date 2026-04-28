#pragma once

#include <string>
#include <vector>
#include "Mesh.h"

struct aiScene;
struct aiNode;
struct aiMesh;

class Modelo {
public:
    std::vector<Malha> malhas;
    std::string        caminhoArquivo;
    std::string        diretorio;

    Modelo() = default;
    explicit Modelo(const std::string& caminho);

    void desenhar() const;

private:
    void processarNo(const aiNode* no, const aiScene* cena);
    Malha processarMalha(const aiMesh* malha, const aiScene* cena);
};
