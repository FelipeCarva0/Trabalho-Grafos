#ifndef __IFT_HPP__
#define __IFT_HPP__

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>

using namespace std;
using namespace cv;

class IFT {

    public:
        enum class ColorMode {
            RGB,
            GRAYSCALE,
            MEAN_COLOR
        };

    private: 
        struct Vertice {
            int row, col;
            Vec3b color; // RGB
            bool isSeed = false;
        };

        struct Edge {
            int v1, v2;
            int weight;

            Edge(int v1, int v2, int weight)
            {
                this->v1 = v1;
                this->v2 = v2;
                this->weight = weight;
            }
        };

        int n; // Número de sementes

        string imagePath;

        Mat image;
        Mat segmentedImage;

        vector<Vertice> vertices;
        vector<Edge> edges;

        vector<int> distances; // C(t)
        vector<int> predecessors; // P(t)
        vector<int> labels; // L(t)
        
        vector<vector<pair<int,int>>> adjList; // adjList[v] = lista de {vizinho, peso}

    public:
        IFT(string &imagePath, int n); 
        void buildGraph();
        Mat segment();
        int calculateWeight(const Vertice& v1, const Vertice& v2);
        Mat renderSegments(ColorMode mode);
        
};

#endif // __IFT_HPP__