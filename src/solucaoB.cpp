#include "include/mst.hpp"
#include <iostream>
#include <filesystem>
#include <random>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;
string imagePath = "assets/images/horse.jpg";


//construtor da MST
MST::MST(string &imagePath, float k){
    this->k = k;
    image = imread(imagePath, IMREAD_COLOR);

    if(image.empty()){
        cout << "ERRO AO CARREGAR IMAGEM \n" << endl;
    }
}


//função recebe os vertices do grafo e calcula seu peso basaedo na diferença de cores de um pixel para o outro.
int MST::calculateWeight(const Vertice& v1, const Vertice& v2){
    return abs(v1.color[0] - v2.color[0]) + abs(v1.color[1] - v2.color[1]) + abs(v1.color[2] - v2.color[2]);
}


//Cria o grafo inicial e define os de pesos de suas arestas.
void MST::buildGraph(){
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


//Salva os conjuntos disjuntos que foram mesclados em uma nova imagem
void saveSegmentResult(const Mat& img, const string& imageName, const string& suffix, const string& outDir = "assets/output"){
    fs::path inputPath(imageName);

    string baseName = inputPath.stem().string();
    string ext = inputPath.extension().string();

    if (ext.empty()) {
        ext = ".png"; 
    }

    fs::path dir(outDir);
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }

    fs::path outputPath = dir / (baseName + "_B_" + suffix + ext);

    imwrite(outputPath.string(), img);
}


//Salva a imagem colorida (fiel as cores originais), imagem colorida (cores aleatorias), imagem em tons de cinza.
Mat MST::renderSegments(DisjointSet& ds, int width, int height, ColorMode mode){

    Mat result(height, width, CV_8UC3);
    unordered_map<int, Vec3b> colors;
                
    mt19937 rng(123);
    uniform_int_distribution<int> dist(0, 255);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            int id = y * width + x;
            int root = ds.find(id);

            Vec3b color;

            if (mode == ColorMode::GRAYSCALE) {

                int gray = (root * 2654435761u) % 256;
                color = Vec3b(gray, gray, gray);

            } else {

                auto it = colors.find(root);

                if (it == colors.end()) {
                    Vec3b c(dist(rng), dist(rng), dist(rng));
                    colors[root] = c;
                    it = colors.find(root); 
                }

                color = it->second;
            }

            result.at<Vec3b>(y, x) = color;
        }
    }

    return result;
}

Mat MST::renderSegmentsByMeanColor(DisjointSet& ds, int width, int height) {

    unordered_map<int, Vec3i> colorSum;
    unordered_map<int, int>   pixelCount;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int root = ds.find(y * width + x);
            Vec3b pixel = image.at<Vec3b>(y, x);
            colorSum[root][0] += pixel[0];
            colorSum[root][1] += pixel[1];
            colorSum[root][2] += pixel[2];
            pixelCount[root]  += 1;
        }
    }

    unordered_map<int, Vec3b> meanColor;
    for (auto& [root, sum] : colorSum) {
        int n = pixelCount[root];
        meanColor[root] = Vec3b(sum[0]/n, sum[1]/n, sum[2]/n);
    }

    Mat result(height, width, CV_8UC3);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int root = ds.find(y * width + x);
            result.at<Vec3b>(y, x) = meanColor[root];
        }
    }

    return result;
}


//conjunto de funções auxiliares da função: MST::segment
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
        internal_diff[x] = max(max(internal_diff[x], internal_diff[y]), (float)weight);
    }
}


//Segmenta a imagem em conjuntos disjuntos (inicialmente cada pixel representando um conjunto) e unifica os conjuntos compativeis
Mat MST::segment(){

    buildGraph();

    // Passo 1: ordena arestas por peso
    sort(edges.begin(), edges.end(), [](Edge a, Edge b) {
        return a.weight < b.weight;
    });

    
    // Passo 2: cada vértice começa no próprio componente
    DisjointSet ds(vertices.size());


    float limiar = 35.0f;// define o valor de corte para a diferença de cores


    // Passo 3: Para cada aresta, verificar se os vértices pertencem a componentes diferentes, e os une caso sim.
    for(const Edge& edge : edges){
    if (edge.weight > limiar) {
            break;
        } 
       
        int x = ds.find(edge.v1); 
        int y = ds.find(edge.v2);

        if(x == y){
            continue;
        }

            ds.unite(x, y, edge.weight);
        
    }

    Mat mean = renderSegmentsByMeanColor(ds, image.cols, image.rows);
    saveSegmentResult(mean, imagePath, "mean");

    Mat rgb = renderSegments(ds, image.cols, image.rows, MST::ColorMode::RGB);
    saveSegmentResult(rgb, imagePath, "rgb");

    Mat gray = renderSegments(ds, image.cols, image.rows, MST::ColorMode::GRAYSCALE);
    saveSegmentResult(gray, imagePath, "gray");

    segmentedImage = rgb;

    // Passo 4: Retornar a segmentação resultante
    return segmentedImage;
}

int main() {
    std::cout << "Solução B" << std::endl;
    MST mst(imagePath, 0);//o segundo valor não é utilizado na alternativa B.
    mst.segment();
    
    return 0;
} 