#include "include/mst.hpp"
#include <iostream>
#include <filesystem>

using namespace std;
using namespace cv;

MST::DisjointSet::DisjointSet(int n){
    parent.resize(n);
    size.resize(n, 1);
    internal_diff.resize(n, 0.0f);

    for (int i = 0; i < n; i++) {
        parent[i] = i;
    }
}

int MST::DisjointSet::find(int u){
    if(parent[u] == u){
        return u;
    }

    return find(parent[u]);
}

void MST::DisjointSet::unite(int u, int v){
    int x = find(u);
    int y = find(v);

    if (x != y){
        if(size[y] < size[x]){
            swap(x, y);
        }

        parent[x] = y;
    }
}

MST::MST(string &imagemPath, float k){
    imagem = imread(imagemPath, IMREAD_COLOR);

    if(imagem.empty()){
        cout << "ERRO AO CARREGAR IMAGEM \n" << endl;
    }
}

int MST::calcularPeso(Vertice v1, Vertice v2) {
    return abs(v1.cor[0] - v2.cor[0]) + abs(v1.cor[1] - v2.cor[1]) + abs(v1.cor[2] - v2.cor[2]);
}

void MST::construirGrafo(){ // grafo construindo com o metodo "Grid Graphs" sessão 5 do artigo
    int lin = imagem.rows;
    int col = imagem.cols;
    int numeroVertices = lin*col;

    vertices.resize(numeroVertices);
    arestas.clear();

    for(int i = 0; i < lin; i++){
        for(int j = 0; j < col; j++){
            vertices[i*col + j] = {i, j, imagem.at<Vec3b>(i, j)};
        }
    }

    for(int i = 0; i < lin; i++){
        for(int j = 0; j < col; j++){

            if(j < col - 1){
                int peso = calcularPeso(vertices[i*col + j], vertices[i*col + (j+1)]);
                arestas.push_back({i*col + j, i*col + (j+1), peso});
            }
            if(i < lin - 1){
                int peso = calcularPeso(vertices[i*col + j], vertices[(i+1)*col + j]);
                arestas.push_back({i*col + j, (i+1)*col + j, peso});
            }
        }
    }
}

Mat MST::segmentacao(){

    construirGrafo();

    // Passo 0: ordena arestas por peso
    sort(arestas.begin(), arestas.end(), [](Aresta a, Aresta b) {
        return a.peso < b.peso;
    });

    // Passo 1: cada vértice começa no próprio componente
    DisjointSet ds(vertices.size());

    // Passo 2: Repitir o passo 3 até que todas as arestas sejam processadas


    // Passo 4: Retornar a segmentação resultante
    return imagem; //por enquanto, depois tem que retornar a imagem segmentada
}

int main() {
    string imagemPath = "assets/images/predio.jpg";
    MST mst(imagemPath, 0.5);
    mst.segmentacao();
    
    return 0;
}