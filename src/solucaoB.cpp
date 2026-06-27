#include <iostream>
#include <filesystem>
#include <random>
#include "include/utils.hpp"
#include "include/mstB.hpp" 

using namespace std;
using namespace cv;
namespace fs = std::filesystem;
//string imagePath = "assets/images/lioness.jpg";

//construtor da MSTB
MSTB::MSTB(string &imagePath, float k){
    this->imagePath = imagePath;
    this->k = k;
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

    cout << "MST criada com "
         << mstEdges.size()
         << " arestas" << endl;
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

    return renderSegmentsByMeanColor(
                ds,
                image.cols,
                image.rows);
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

        string rootDir =
            "assets/output/B/" +
            baseName;

        saveSegmentResultSolucaoB(
            renderSegments(
                ds,
                image.cols,
                image.rows,
                ColorMode::RGB),
            imagePath,
            "lambda_" + to_string(lambda),
            rootDir + "/rgb"
        );

        saveSegmentResultSolucaoB(
            renderSegmentsByMeanColor(
                ds,
                image.cols,
                image.rows),
            imagePath,
            "lambda_" + to_string(lambda),
            rootDir + "/mean"
        );

        saveSegmentResultSolucaoB(
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

//Salva a imagem colorida (fiel as cores originais), imagem colorida (cores aleatorias), imagem em tons de cinza.
Mat MSTB::renderSegments(DisjointSet& ds, int width, int height, ColorMode mode){

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


//Renderiza a segmentação usando a cor média de cada segmento
Mat MSTB::renderSegmentsByMeanColor(DisjointSet& ds, int width, int height) {

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
        internal_diff[x] = max(max(internal_diff[x], internal_diff[y]), (float)weight);
    }
}


//Segmenta a imagem em conjuntos disjuntos (inicialmente cada pixel representando um conjunto) e unifica os conjuntos compativeis
//Executa o pipeline completo de segmentação
Mat MSTB::segment()
{
    buildGraph();

    buildMST();

    saveHierarchy();

    Mat saliency =
        computeSaliencyMap();

    fs::path inputPath(imagePath);

    string baseName =
        inputPath.stem().string();

    string ext =
        inputPath.extension().string();

    string output =
        "assets/output/B/" +
        baseName +
        "_saliency" +
        ext;

    imwrite(output,saliency);

    segmentedImage =
        computeQFZ(40);

    return segmentedImage;
}

/*int main() {
    std::cout << "Solução B" << std::endl;
    MSTB mstB(imagePath, 0);//o segundo valor não é utilizado na alternativa B.
    mstB.segment();
    
    return 0;
} */
