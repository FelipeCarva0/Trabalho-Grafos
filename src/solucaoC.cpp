#include "include/ift.hpp"
#include <iostream>
#include <filesystem>
#include <random>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

string imagePath = "assets/images/horse.jpg";

IFT::IFT(string &imagePath, int n){
    this->n = n;
    image = imread(imagePath, IMREAD_COLOR);

    if(image.empty()){
        cout << "ERRO AO CARREGAR IMAGEM \n" << endl;
    }
}

int IFT::calculateWeight(const Vertice& v1, const Vertice& v2){
    return abs(v1.color[0] - v2.color[0]) + abs(v1.color[1] - v2.color[1]) + abs(v1.color[2] - v2.color[2]);
}

/*int IFT::calculateWeight(const Vertice& v1, const Vertice& v2){
    return sqrt(pow(v1.color[0] - v2.color[0], 2) + pow(v1.color[1] - v2.color[1], 2) + pow(v1.color[2] - v2.color[2], 2));
}*/

void IFT::buildGraph(){ 
    int row = image.rows;
    int col = image.cols;
    int numeroVertices = row*col;

    vertices.resize(numeroVertices);
    edges.clear();

    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            vertices[i*col + j] = {i, j, image.at<Vec3b>(i, j)};
            // função para definir se sera semente
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

int main() {
    IFT ift(imagePath, 120);
    ift.buildGraph();
    return 0;
}