#include <iostream>
#include <filesystem>
#include <random>
#include <climits>
#include <cmath>
#include "include/utils.hpp"
#include "include/ift.hpp"

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

//string imagePath = "assets/images/lioness.jpg";

//--- IFT ---

IFT::IFT(string &imagePath, int n){
    this->n = n;
    image = imread(imagePath, IMREAD_COLOR);
    this->imagePath = imagePath;

    if(image.empty()){
        cout << "ERRO AO CARREGAR IMAGEM \n" << endl;
    }
}

int IFT::calculateWeight(const Vertice& v1, const Vertice& v2){
    return abs(v1.color[0] - v2.color[0]) + abs(v1.color[1] - v2.color[1]) + abs(v1.color[2] - v2.color[2]);
}

Mat IFT::segment() { // Algoritmo 3
    buildGraph();

    int num_vertices = image.rows * image.cols;

    distances.assign(num_vertices, INT_MAX);
    predecessors.assign(num_vertices, -1);
    labels.assign(num_vertices, -1);

    // Fila LIFO: (custo, -ordem, pixel)
    using Entry = tuple<int, long, int>;
    priority_queue<Entry, vector<Entry>, greater<Entry>> priorityQueue;

    vector<bool> inF(num_vertices, false); // inF[i] = true; pixel i já processado definitivamente
    vector<bool> inQueue(num_vertices, false); // inQueue[i] = true; pixel i está atualmente na fila (para o lazy deletion)
    long order = 0;

    // Passo 1: Inicializar as estruturas de dados
    for (int i = 0; i < num_vertices; ++i) {
        if (vertices[i].isSeed) {
            distances[i] = 0;
            labels[i] = i;
            priorityQueue.push({0, -(order++), i});
            inQueue[i] = true;
        }
    }

    // Passo 2: Enquanto a fila de prioridade não estiver vazia
    while (!priorityQueue.empty()) {
        // Passo 2.1: Extrair o vértice com a menor distância (C(t))
        auto [cost, neg_ord, s] = priorityQueue.top();
        priorityQueue.pop();

        if (!inQueue[s] || cost != distances[s]) continue;

        inQueue[s] = false;
        inF[s] = true;

        // Passo 2.2: Para cada vértice adjacente ao vértice atual
        for (auto& [t, w] : adjList[s]) {

            // Pixel já em F: nunca atualiza
            if (inF[t]){
                continue;
            }

            // Passo 2.2.1: Calcular o custo de alcançar o vértice adjacente através do vértice atual
            // f_peak: custo = max ao longo do caminho
            int newCost = max(distances[s], w);

            // Passo 2.2.2: Se o custo calculado for menor do que a distância atualmente conhecida para o vértice adjacente, 
            // atualizar a distância, o predecessor e o rótulo do vértice adjacente
            if (newCost < distances[t]) {
                inQueue[t] = false; 
                distances[t] = newCost;
                predecessors[t] = s;
                labels[t] = labels[s];
                priorityQueue.push({newCost, -(order++), t});
                inQueue[t] = true;
            }
        }
    }

    Mat meanColorImage = renderSegments(IFT::ColorMode::MEAN_COLOR);
    saveSegmentResult(meanColorImage, imagePath, "mean", "assets/output/C");

    Mat rgbImage = renderSegments(IFT::ColorMode::RGB);
    saveSegmentResult(rgbImage, imagePath, "rgb", "assets/output/C");

    Mat grayscaleImage = renderSegments(IFT::ColorMode::GRAYSCALE);
    saveSegmentResult(grayscaleImage, imagePath, "grayscale", "assets/output/C");

    segmentedImage = rgbImage;

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
    adjList.assign(numeroVertices, {});  

    for (int i = 0; i < row; i++){
        for (int j = 0; j < col; j++){
            vertices[i*col + j] = {i, j, image.at<Vec3b>(i, j)};

            auto mod_seed = [&](int v, int step) {
                int r = (v - step/2) % step;
                if (r < 0) r += step;
                return r;
            };

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

                adjList[u].push_back({v, peso});   
                adjList[v].push_back({u, peso});  
            }
            if(i < row - 1){
                int v = (i+1)*col + j;
                int peso = calculateWeight(vertices[u], vertices[v]);

                edges.push_back({u, v, peso});
                adjList[u].push_back({v, peso});  
                adjList[v].push_back({u, peso});  
            }
        }
    }
}

Mat IFT::renderSegments(ColorMode mode)
{
    const int rows = image.rows;
    const int cols = image.cols;
    const int total = rows * cols;

    Mat result(rows, cols, CV_8UC3);

    switch (mode)
    {
        case ColorMode::GRAYSCALE:
        {
            for (int y = 0; y < rows; y++)
            {
                Vec3b* dst = result.ptr<Vec3b>(y);

                for (int x = 0; x < cols; x++)
                {
                    int label = labels[y * cols + x];
                    uchar gray = static_cast<uchar>((label * 2654435761u) & 255);

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

            for (int y = 0; y < rows; y++)
            {
                Vec3b* dst = result.ptr<Vec3b>(y);

                for (int x = 0; x < cols; x++)
                {
                    int label = labels[y * cols + x];

                    auto [it, inserted] = colors.try_emplace(label);

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

            sums.reserve(total / 4);
            counts.reserve(total / 4);

            // Soma das cores
            for (int y = 0; y < rows; y++)
            {
                const Vec3b* src = image.ptr<Vec3b>(y);

                for (int x = 0; x < cols; x++)
                {
                    int label = labels[y * cols + x];

                    Vec3i& s = sums[label];
                    s[0] += src[x][0];
                    s[1] += src[x][1];
                    s[2] += src[x][2];

                    counts[label]++;
                }
            }

            // Cor média
            unordered_map<int, Vec3b> meanColor;
            meanColor.reserve(sums.size());

            for (const auto& [label, sum] : sums)
            {
                int n = counts[label];

                meanColor.emplace(
                    label,
                    Vec3b(
                        sum[0] / n,
                        sum[1] / n,
                        sum[2] / n
                    )
                );
            }

            // Renderização
            for (int y = 0; y < rows; y++)
            {
                Vec3b* dst = result.ptr<Vec3b>(y);

                for (int x = 0; x < cols; x++)
                {
                    dst[x] = meanColor[labels[y * cols + x]];
                }
            }

            break;
        }
    }

    return result;
}

/*int main() {
    IFT ift(imagePath, 300);
    ift.segment();

    return 0;
}*/