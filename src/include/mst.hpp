#ifndef __MST_HPP__
#define __MST_HPP__

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

using namespace std;
using namespace cv;

class MST {

    enum class ColorMode {
        RGB,
        GRAYSCALE,
        MEAN_COLOR
    };

    private: 
        struct Vertice {
            int row, col;
            Vec3b color; // RGB
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

        
        class DisjointSet {
            private:
                vector<int> parent;
                vector<int> size;
                vector<float> internal_diff;

            public:
                float getInternal_diff(int x);
                int getSize(int x);
                DisjointSet(int n);
                int find(int u);
                void unite(int u, int v, int weight);
        };

        float k;

        string imagePath;

        Mat image;
        Mat segmentedImage;

        vector<Vertice> vertices;
        vector<Edge> edges;

    public:
        MST(string &imagePath, float k); // float K é utilizado na solução A
        Mat renderSegments(DisjointSet& ds, int width, int height, ColorMode mode);
        void buildGraph();
        int calculateWeight(const Vertice& v1, const Vertice& v2);
        float threshold(float k, int size);
        Mat segment();
};

#endif // __MST_HPP__