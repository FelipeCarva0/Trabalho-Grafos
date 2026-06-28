#include "include/mst.hpp"
#include <iostream>
#include <filesystem>
#include <random>
#include "include/utils.hpp"
#include "include/mst.hpp"

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

//string imagePath = "assets/images/lioness.jpg";

//--- Disjoint Set ---

MST::DisjointSet::DisjointSet(int n){
    parent.resize(n);
    size.resize(n, 1);
    internal_diff.resize(n, 0.0f);

    for (int i = 0; i < n; i++) {
        parent[i] = i;
    }
}

int MST::DisjointSet::find(int u){
    if(parent[u] != u){
        parent[u] = find(parent[u]);
    }
    return parent[u];
}

void MST::DisjointSet::unite(int u, int v, int weight){
    int x = find(u);
    int y = find(v);

    if (x != y){
        if (size[x] < size[y]) {
            swap(x, y);
        }

        parent[y] = x;
        size[x] += size[y];
        internal_diff[x] = max({internal_diff[x], internal_diff[y], (float)weight});
    }
}

float MST::DisjointSet::getInternal_diff(int x){
    return internal_diff[find(x)];
}

int MST::DisjointSet::getSize(int x){
    return size[find(x)];
}

// --- MST ---

MST::MST(string &imagePath, float k){
    this->k = k;
    image = imread(imagePath, IMREAD_COLOR);

    this->imagePath = imagePath;

    if(image.empty()){
        cout << "ERRO AO CARREGAR IMAGEM \n" << endl;
    }
}

int MST::calculateWeight(const Vertice& v1, const Vertice& v2){
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

float MST::threshold(float k, int size){
    return k/size;
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
    for(const Edge& edge : edges){

        // Passo 3: Para cada aresta, verificar se os vértices pertencem a componentes diferentes
        int x = ds.find(edge.v1); 
        int y = ds.find(edge.v2);

        if(x == y){
            continue;
        }

        int mintX = (ds.getInternal_diff(x) + threshold(k, ds.getSize(x)));
        int mintY = (ds.getInternal_diff(y) + threshold(k, ds.getSize(y)));

        if(edge.weight <= min(mintX, mintY)){
            ds.unite(x, y, edge.weight);
        }
    }

    Mat rgb = renderSegments(ds, image.cols, image.rows, MST::ColorMode::RGB);
    saveSegmentResult(rgb, imagePath, "rgb", "assets/output/A");

    Mat gray = renderSegments(ds, image.cols, image.rows, MST::ColorMode::GRAYSCALE);
    saveSegmentResult(gray, imagePath, "gray", "assets/output/A");

    Mat mean = renderSegments(ds, image.cols, image.rows, MST::ColorMode::MEAN_COLOR);
    saveSegmentResult(mean, imagePath, "mean", "assets/output/A");

    segmentedImage = rgb;

    // Passo 4: Retornar a segmentação resultante
    return segmentedImage;
}

Mat MST::renderSegments(DisjointSet& ds, int width, int height, ColorMode mode){
    Mat result(height, width, CV_8UC3);

    const int total = width * height;

    // Guarda o representante de cada pixel
    vector<int> roots(total);
    for (int i = 0; i < total; i++)
        roots[i] = ds.find(i);

    switch (mode)
    {
        case ColorMode::GRAYSCALE:
        {
            for (int y = 0; y < height; y++)
            {
                Vec3b* dst = result.ptr<Vec3b>(y);

                for (int x = 0; x < width; x++)
                {
                    int id = y * width + x;
                    int gray = (roots[id] * 2654435761u) & 255;

                    dst[x] = Vec3b(gray, gray, gray);
                }
            }

            break;
        }

        case ColorMode::RGB:
        {
            unordered_map<int, Vec3b> colors;
            colors.reserve(total / 4);

            mt19937 rng(123);
            uniform_int_distribution<int> dist(0, 255);

            for (int y = 0; y < height; y++)
            {
                Vec3b* dst = result.ptr<Vec3b>(y);

                for (int x = 0; x < width; x++)
                {
                    int id = y * width + x;
                    int root = roots[id];

                    auto [it, inserted] = colors.try_emplace(root);

                    if (inserted)
                    {
                        it->second = Vec3b(
                            dist(rng),
                            dist(rng),
                            dist(rng)
                        );
                    }

                    dst[x] = it->second;
                }
            }

            break;
        }

        case ColorMode::MEAN_COLOR:
        {
            unordered_map<int, Vec3i> sums;
            unordered_map<int, int> counts;
            unordered_map<int, Vec3b> meanColors;

            sums.reserve(total / 4);
            counts.reserve(total / 4);
            meanColors.reserve(total / 4);

            // Soma das cores
            for (int y = 0; y < height; y++)
            {
                const Vec3b* src = image.ptr<Vec3b>(y);

                for (int x = 0; x < width; x++)
                {
                    int id = y * width + x;
                    int root = roots[id];

                    Vec3i& s = sums[root];

                    s[0] += src[x][0];
                    s[1] += src[x][1];
                    s[2] += src[x][2];

                    counts[root]++;
                }
            }

            // Calcula média
            for (const auto& p : sums)
            {
                int root = p.first;
                const Vec3i& s = p.second;
                int n = counts[root];

                meanColors[root] = Vec3b(
                    s[0] / n,
                    s[1] / n,
                    s[2] / n
                );
            }

            // Gera imagem
            for (int y = 0; y < height; y++)
            {
                Vec3b* dst = result.ptr<Vec3b>(y);

                for (int x = 0; x < width; x++)
                {
                    int id = y * width + x;
                    dst[x] = meanColors[roots[id]];
                }
            }

            break;
        }
    }

    return result;
}

/*int main(){
    MST mst(imagePath, 30000.0f);
    mst.segment();
    
    return 0;
}*/