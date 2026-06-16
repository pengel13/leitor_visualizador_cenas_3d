#pragma once

#include <string>
#include <vector>

#include "Mesh.h"

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;

// Modelo carrega um arquivo 3D (OBJ, PLY, FBX, etc.) via Assimp
// e o decompoe em uma lista de Malha. Cada Malha corresponde a um
// grupo de faces do modelo e possui seu proprio material e textura
// lidos do arquivo .mtl associado.
class Modelo {
public:
    std::vector<Malha> malhas;
    std::string        caminhoArquivo;
    std::string        diretorio;    // diretorio do arquivo; usado para resolver caminhos de textura

    // Construtor padrao: cria um modelo vazio (usado por Primitivos)
    Modelo() = default;

    // Carrega um modelo do disco usando Assimp.
    // Lanca std::runtime_error se o arquivo nao for encontrado ou invalido.
    explicit Modelo(const std::string& caminho);

    // Desenha todas as malhas sem material (usado pelo wireframe overlay)
    void desenhar() const;

    // Desenha todas as malhas com upload de material e textura para o shader
    void desenhar(Shader& shader) const;

private:
    // Percorre a arvore de nos do Assimp recursivamente, convertendo cada malha
    void processarNo(const aiNode* no, const aiScene* cena);

    // Converte uma aiMesh do Assimp em uma Malha do nosso formato
    Malha processarMalha(const aiMesh* malha, const aiScene* cena);

    // Le as propriedades de material (ka, kd, ks, brilho, textura difusa)
    // do aiMaterial do Assimp e preenche os campos da Malha
    void carregarMaterial(Malha& malha, const aiMaterial* mat);
};
