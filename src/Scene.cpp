#include "Scene.h"

#include <iostream>
#include <algorithm>

void Cena::adicionarObjeto(std::unique_ptr<Objeto3D> obj) {
    objetos.push_back(std::move(obj));
    sincronizarFlagsSelecao();
}

void Cena::selecionarProximo() {
    if (objetos.empty()) return;
    indiceAtivo = (indiceAtivo + 1) % static_cast<int>(objetos.size());
    sincronizarFlagsSelecao();
    std::cout << "[Cena] Selecionado: " << objetos[indiceAtivo]->nome
              << " (" << indiceAtivo + 1 << "/" << objetos.size() << ")\n";
}

void Cena::selecionarAnterior() {
    if (objetos.empty()) return;
    indiceAtivo = (indiceAtivo - 1 + static_cast<int>(objetos.size()))
                  % static_cast<int>(objetos.size());
    sincronizarFlagsSelecao();
    std::cout << "[Cena] Selecionado: " << objetos[indiceAtivo]->nome
              << " (" << indiceAtivo + 1 << "/" << objetos.size() << ")\n";
}

Objeto3D* Cena::obterAtivo() {
    if (objetos.empty()) return nullptr;
    return objetos[indiceAtivo].get();
}

const Objeto3D* Cena::obterAtivo() const {
    if (objetos.empty()) return nullptr;
    return objetos[indiceAtivo].get();
}

void Cena::imprimirStatus() const {
    if (objetos.empty()) {
        std::cout << "[Cena] Vazia\n";
        return;
    }
    const Objeto3D* a = obterAtivo();
    std::cout << "[Cena] Objetos: " << objetos.size()
              << "  Ativo: " << a->nome
              << "  Pos: (" << a->posicao.x << ", "
                            << a->posicao.y << ", "
                            << a->posicao.z << ")\n";
}

void Cena::sincronizarFlagsSelecao() {
    for (int i = 0; i < static_cast<int>(objetos.size()); ++i) {
        objetos[i]->estaSelecionado = (i == indiceAtivo);
    }
}
