#include "include/ift.hpp"
#include <iostream>
#include <filesystem>
#include <random>
#include <climits>
#include <cmath>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

string imagePath = "assets/images/building.jpg";

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

Mat IFT::segment(){ // Algoritmo 3 do artigo
    buildGraph();

    int num_vertices = image.rows * image.cols;

    // Passo 1: Inicializar as estruturas de dados
    distances.assign(num_vertices, INT_MAX);
    predecessors.assign(num_vertices, -1);
    labels.assign(num_vertices, -1);

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> priorityQueue; // (distance, vertex)

    for (int i = 0; i < num_vertices; ++i)
	{
		if (vertices[i].isSeed)
		{
			distances[i] = 0;
			labels[i] = i;
            priorityQueue.push({0, i});
		}
	}

    // Passo 2: Enquanto a fila de prioridade não estiver vazia
    while (!priorityQueue.empty()) {
        pair<int, int> topElement = priorityQueue.top();
        priorityQueue.pop();

        int cost = topElement.first;
        int current = topElement.second;

        if(cost > distances[current]){
            continue;
        }

        // Agora só percorre os vizinhos reais de "current"
        for(const auto& [neighbor, weight] : adjList[current]){
            int newCost = distances[current] + weight;

            if(newCost < distances[neighbor]) {
                distances[neighbor] = newCost;
                predecessors[neighbor] = current;
                labels[neighbor] = labels[current];

                priorityQueue.push({newCost, neighbor});
            }
        }
    }


    segmentedImage = renderSegments(ColorMode::RGB);

    return segmentedImage;
}

void IFT::buildGraph(){ 
    int row = image.rows;
    int col = image.cols;
    int numeroVertices = row*col;

    int step = sqrt((double)numeroVertices/n);
    if (step < 1)
        step = 1;

    vertices.resize(numeroVertices);
    edges.clear();
    adjList.assign(numeroVertices, {});   // <-- novo

    for (int i = 0; i < row; i++){
        for (int j = 0; j < col; j++){
            vertices[i*col + j] = {i, j, image.at<Vec3b>(i, j)};

            auto mod_seed = [&](int v, int step) {
                int r = (v - step/2) % step;
                if (r < 0) r += step;
                return r;
            };

            // dentro do loop:
            if (mod_seed(i, step) == 0 && mod_seed(j, step) == 0){
                vertices[i*col + j].isSeed = true;
            }
        }
    }

    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            int u = i*col + j;

            if(j < col - 1){
                int v = i*col + (j+1);
                int peso = calculateWeight(vertices[u], vertices[v]);
                edges.push_back({u, v, peso});
                adjList[u].push_back({v, peso});   // <-- novo
                adjList[v].push_back({u, peso});   // <-- novo
            }
            if(i < row - 1){
                int v = (i+1)*col + j;
                int peso = calculateWeight(vertices[u], vertices[v]);
                edges.push_back({u, v, peso});
                adjList[u].push_back({v, peso});   // <-- novo
                adjList[v].push_back({u, peso});   // <-- novo
            }
        }
    }
}

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

    fs::path outputPath = dir / (baseName + "_" + suffix + ext);

    imwrite(outputPath.string(), img);
}

Mat IFT::renderSegments(ColorMode mode){
    Mat result(image.rows, image.cols, CV_8UC3);

    unordered_map<int, Vec3b> colors;

    mt19937 rng(123);
    uniform_int_distribution<int> dist(0, 255);

    for(int y = 0; y < image.rows; y++)
    {
        for(int x = 0; x < image.cols; x++)
        {
            int id = y * image.cols + x;

            int label = labels[id];

            Vec3b color;

            if(mode == ColorMode::GRAYSCALE)
            {
                int gray =
                    (label * 2654435761u) % 256;

                color = Vec3b(gray, gray, gray);
            }
            else
            {
                auto it = colors.find(label);

                if(it == colors.end())
                {
                    Vec3b c(
                        dist(rng),
                        dist(rng),
                        dist(rng)
                    );

                    colors[label] = c;
                    it = colors.find(label);
                }

                color = it->second;
            }

            result.at<Vec3b>(y, x) = color;
        }
    }

    saveSegmentResult(result, imagePath, "rgb");

    return result;
}

int main() {
    IFT ift(imagePath, 350);
    ift.segment();

    return 0;
}