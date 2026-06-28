#include "include/mstB.hpp"

#include <iostream>
#include <filesystem>
#include <random>
#include "include/utils.hpp"
#include "include/mstB.hpp" 

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

//construtor da MSTB
MSTB::MSTB(string &imagePath){
    this->imagePath = imagePath;

    image = imread(imagePath, IMREAD_COLOR);

    if(image.empty()){
        cout << "ERRO AO CARREGAR IMAGEM \n" << endl;
    }
}

//função recebe os vertices do grafo e calcula seu peso basaedo na diferença de cores de um pixel para o outro.
int MSTB::calculateWeight(
    const Vertice& v1,
    const Vertice& v2)
{
    int db =
        v1.color[0]-v2.color[0];

    int dg =
        v1.color[1]-v2.color[1];

    int dr =
        v1.color[2]-v2.color[2];

    return sqrt(
        db*db +
        dg*dg +
        dr*dr);
}

//Cria o grafo inicial e define os de pesos de suas arestas.
void MSTB::buildGraph(){
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

//brief Constrói a Árvore Geradora Mínima (MST) do grafo
void MSTB::buildMST()
{
    mstEdges.clear();

    sort(edges.begin(),
         edges.end(),
         [](const Edge& a, const Edge& b)
         {
             return a.weight < b.weight;
         });

    DisjointSet ds(vertices.size());

    for(const Edge& e : edges)
    {
        int x = ds.find(e.v1);
        int y = ds.find(e.v2);

        if(x != y)
        {
            mstEdges.push_back(e);
            ds.unite(x,y,e.weight);
        }
    }
}

//Computa a segmentação usando o limiar lambda
Mat MSTB::computeQFZ(float lambda)
{
    DisjointSet ds(vertices.size());

    for(const Edge& e : mstEdges)
    {
        if(e.weight > lambda)
            break;

        ds.unite(e.v1,e.v2,e.weight);
    }

    return renderSegments(
                ds,
                image.cols,
                image.rows,
                ColorMode::RGB);
}

//Computa a segmentação usando o limiar lambda
Mat MSTB::computeSaliencyMap()
{
    Mat saliency(
        image.rows,
        image.cols,
        CV_8UC1,
        Scalar(0));

    int maxWeight = 1;

    for(const Edge& e : mstEdges)
        maxWeight =
            max(maxWeight,e.weight);

    for(const Edge& e : mstEdges)
    {
        int value =
            (255 * e.weight) / maxWeight;

        auto& v1 = vertices[e.v1];
        auto& v2 = vertices[e.v2];

        saliency.at<uchar>(
            v1.row,
            v1.col)
        =
        max(
            saliency.at<uchar>(
                v1.row,
                v1.col),
            (uchar)value);

        saliency.at<uchar>(
            v2.row,
            v2.col)
        =
        max(
            saliency.at<uchar>(
                v2.row,
                v2.col),
            (uchar)value);
    }

    return saliency;
}

//Salva hierarquia de segmentações para diferentes limiares
void MSTB::saveHierarchy()
{
    vector<int> levels =
    {
        1,
        2,
        3,
        5,
        10,
        15,
        20,
        30,
        40,
        50,
    };

    fs::path inputPath(imagePath);
    string baseName = inputPath.stem().string();

    for(int lambda : levels)
    {
        DisjointSet ds(vertices.size());

        for(const Edge& e : mstEdges)
        {
            if(e.weight > lambda)
                break;

            ds.unite(e.v1, e.v2, e.weight);
        }

        string rootDir = "assets/output/B/" + baseName;

        saveSegmentResult(
            renderSegments(
                ds,
                image.cols,
                image.rows,
                ColorMode::RGB),
            imagePath,
            "lambda_" + to_string(lambda),
            rootDir + "/rgb"
        );

        saveSegmentResult(
            renderSegments(
                ds,
                image.cols,
                image.rows,
                ColorMode::MEAN_COLOR),
            imagePath,
            "lambda_" + to_string(lambda),
            rootDir + "/mean"
        );

        saveSegmentResult(
            renderSegments(
                ds,
                image.cols,
                image.rows,
                ColorMode::GRAYSCALE),
            imagePath,
            "lambda_" + to_string(lambda),
            rootDir + "/gray"
        );
    }
}

//Salva a imagem colorida (fiel as cores originais), imagem colorida (cores aleatorias), 
//imagem em tons de cinza e imagem com a cor média de cada segmento.
Mat MSTB::renderSegments(DisjointSet& ds, int width, int height, ColorMode mode)
{
    const int total = width * height;

    Mat result(height, width, CV_8UC3);

    // Calcula o representante de cada pixel apenas uma vez
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
                    int root = roots[y * width + x];

                    uchar gray = static_cast<uchar>(
                        (root * 2654435761u) & 255
                    );

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
                    int root = roots[y * width + x];

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

            sums.reserve(total / 4);
            counts.reserve(total / 4);

            // Soma das cores de cada segmento
            for (int y = 0; y < height; y++)
            {
                const Vec3b* src = image.ptr<Vec3b>(y);

                for (int x = 0; x < width; x++)
                {
                    int root = roots[y * width + x];

                    Vec3i& s = sums[root];

                    s[0] += src[x][0];
                    s[1] += src[x][1];
                    s[2] += src[x][2];

                    counts[root]++;
                }
            }

            unordered_map<int, Vec3b> meanColor;
            meanColor.reserve(sums.size());

            for (const auto& [root, sum] : sums)
            {
                int n = counts[root];

                meanColor.emplace(
                    root,
                    Vec3b(
                        sum[0] / n,
                        sum[1] / n,
                        sum[2] / n
                    )
                );
            }

            // Renderização
            for (int y = 0; y < height; y++)
            {
                Vec3b* dst = result.ptr<Vec3b>(y);

                for (int x = 0; x < width; x++)
                {
                    dst[x] = meanColor[roots[y * width + x]];
                }
            }

            break;
        }
    }

    return result;
}

//conjunto de funções auxiliares da função: MSTB::segment
MSTB::DisjointSet::DisjointSet(int n){
    parent.resize(n);
    size.resize(n, 1);
    internal_diff.resize(n, 0.0f);

    for (int i = 0; i < n; i++) {
        parent[i] = i;
    }
}
int MSTB::DisjointSet::find(int u){
    if(parent[u] != u){
        parent[u] = find(parent[u]);
    }
    return parent[u];
}
void MSTB::DisjointSet::unite(int u, int v, int weight){
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

//Segmenta a imagem em conjuntos disjuntos (inicialmente cada pixel representando um conjunto) e unifica os conjuntos compativeis
//Executa o pipeline completo de segmentação
Mat MSTB::segment()
{
    buildGraph();

    buildMST();

    saveHierarchy();

    Mat saliency = computeSaliencyMap();

    fs::path inputPath(imagePath);

    string baseName = inputPath.stem().string();

    string ext = inputPath.extension().string();

    string output = "assets/output/B/" + baseName + "_saliency" + ext;

    imwrite(output,saliency);

    segmentedImage = computeQFZ(Lambda);

    return segmentedImage;
}
