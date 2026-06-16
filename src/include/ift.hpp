#ifndef __IFT_HPP__
#define __IFT_HPP__

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

using namespace std;
using namespace cv;

class IFT {

    enum class ColorMode {
        RGB,
        GRAYSCALE
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

        Mat image;
        Mat segmentedImage;

        vector<Vertice> vertices;
        vector<Edge> edges;

        vector<int> seeds;
        vector<int> distances;

    public:
        IFT(string &imagePath, int n); 
        void buildGraph();
        int calculateWeight(const Vertice& v1, const Vertice& v2);
};

#endif // __IFT_HPP__