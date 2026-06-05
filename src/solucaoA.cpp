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
        if (size[y] < size[x]) {
            swap(x, y);
        }

        parent[x] = y;
        size[y] += size[x];
    }
}

MST::MST(string &imagePath, float k){
    image = imread(imagePath, IMREAD_COLOR);

    if(image.empty()){
        cout << "ERRO AO CARREGAR IMAGEM \n" << endl;
    }
}

int MST::calculateWeight(Vertice v1, Vertice v2) {
    return abs(v1.color[0] - v2.color[0]) + abs(v1.color[1] - v2.color[1]) + abs(v1.color[2] - v2.color[2]);
}

void MST::buildGraph(){ // grafo construindo com o metodo "Grid Graphs" sessão 5 do artigo
    int row = image.rows;
    int col = image.cols;
    int numeroVertices = row*col;

    vertices.resize(numeroVertices);
    edges.clear();

    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            vertices[i*col + j] = {i, j, image.at<Vec3b>(i, j)};
        }
    }

    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){

            if(j < col - 1){
                int peso = calculateWeight(vertices[i*col + j], vertices[i*col + (j+1)]);
                edges.push_back({i*col + j, i*col + (j+1), peso});
            }
            if(i < row - 1){
                int peso = calculateWeight(vertices[i*col + j], vertices[(i+1)*col + j]);
                edges.push_back({i*col + j, (i+1)*col + j, peso});
            }
        }
    }
}

Mat MST::segment(){

    buildGraph();

    // Passo 0: ordena arestas por peso
    sort(edges.begin(), edges.end(), [](Edge a, Edge b) {
        return a.weight < b.weight;
    });

    // Passo 1: cada vértice começa no próprio componente
    DisjointSet ds(vertices.size());

    // Passo 2: Repitir o passo 3 até que todas as arestas sejam processadas


    // Passo 4: Retornar a segmentação resultante
    return image; //por enquanto, depois tem que retornar a imagem segmentada
}

int main() {
    string imagePath = "assets/images/building.jpg";
    MST mst(imagePath, 0.5);
    mst.segment();
    
    return 0;
}