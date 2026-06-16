#include "Model.h"
#include "Texture.h"

#include <iostream>
#include <stdexcept>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

Modelo::Modelo(const std::string& caminho) : caminhoArquivo(caminho) {
    std::cout << "[Modelo] Carregando: " << caminho << "\n";

    Assimp::Importer importer;

    const aiScene* cena = importer.ReadFile(caminho,
        aiProcess_Triangulate           |
        aiProcess_GenSmoothNormals      |
        aiProcess_FlipUVs               |
        aiProcess_JoinIdenticalVertices |
        aiProcess_OptimizeMeshes        |
        aiProcess_ValidateDataStructure
    );

    if (!cena || (cena->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !cena->mRootNode) {
        throw std::runtime_error(
            std::string("[Modelo] Erro Assimp: ") + importer.GetErrorString());
    }

    // diretorio do arquivo para resolver caminhos relativos de textura
    diretorio = caminho.substr(0, caminho.find_last_of("/\\"));
    processarNo(cena->mRootNode, cena);

    std::cout << "[Modelo] " << malhas.size() << " malha(s) carregada(s): " << caminho << "\n";
}

void Modelo::processarNo(const aiNode* no, const aiScene* cena) {
    for (unsigned int i = 0; i < no->mNumMeshes; ++i) {
        aiMesh* malha = cena->mMeshes[no->mMeshes[i]];
        malhas.push_back(processarMalha(malha, cena));
    }

    for (unsigned int i = 0; i < no->mNumChildren; ++i) {
        processarNo(no->mChildren[i], cena);
    }
}

Malha Modelo::processarMalha(const aiMesh* malha, const aiScene* cena) {
    std::vector<Vertice>      vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(malha->mNumVertices);
    indices.reserve(static_cast<size_t>(malha->mNumFaces) * 3);

    for (unsigned int i = 0; i < malha->mNumVertices; ++i) {
        Vertice v;

        v.posicao = {
            malha->mVertices[i].x,
            malha->mVertices[i].y,
            malha->mVertices[i].z
        };

        if (malha->HasNormals()) {
            v.normal = {
                malha->mNormals[i].x,
                malha->mNormals[i].y,
                malha->mNormals[i].z
            };
        } else {
            v.normal = {0.f, 1.f, 0.f}; // fallback
        }

        if (malha->mTextureCoords[0]) {
            v.coordTex = {
                malha->mTextureCoords[0][i].x,
                malha->mTextureCoords[0][i].y
            };
        } else {
            v.coordTex = {0.f, 0.f};
        }

        vertices.push_back(v);
    }

    for (unsigned int i = 0; i < malha->mNumFaces; ++i) {
        const aiFace& face = malha->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    Malha m(vertices, indices);

    if (malha->mMaterialIndex < cena->mNumMaterials) {
        carregarMaterial(m, cena->mMaterials[malha->mMaterialIndex]);
    }

    return m;
}

void Modelo::carregarMaterial(Malha& malha, const aiMaterial* mat) {
    aiColor3D cor;

    if (mat->Get(AI_MATKEY_COLOR_AMBIENT, cor) == AI_SUCCESS) {
        malha.material.ambiente = {cor.r, cor.g, cor.b};
    }

    if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, cor) == AI_SUCCESS) {
        malha.material.difuso = {cor.r, cor.g, cor.b};
        malha.material.cor = malha.material.difuso; // kd como albedo quando sem textura
    }

    if (mat->Get(AI_MATKEY_COLOR_SPECULAR, cor) == AI_SUCCESS) {
        malha.material.especular = {cor.r, cor.g, cor.b};
    }

    float brilho = 0.f;
    if (mat->Get(AI_MATKEY_SHININESS, brilho) == AI_SUCCESS && brilho > 0.f) {
        malha.material.brilho = brilho;
    }

    if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString camTextura;
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &camTextura) == AI_SUCCESS) {
            std::string camTexStr = camTextura.C_Str();

            // Assimp pode retornar caminho absoluto ou relativo
            std::string camCompleto;
            if (camTexStr.size() > 1 && (camTexStr[0] == '/' || camTexStr[1] == ':')) {
                camCompleto = camTexStr;
            } else {
                camCompleto = diretorio + "/" + camTexStr;
            }

            malha.textureID = carregarTextura(camCompleto);
        }
    }
}

void Modelo::desenhar() const {
    for (const Malha& m : malhas) {
        m.desenhar();
    }
}

void Modelo::desenhar(Shader& shader) const {
    for (const Malha& m : malhas) {
        m.desenhar(shader);
    }
}
