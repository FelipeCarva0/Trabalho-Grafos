#ifndef __MST_HPP__
#define __MST_HPP__

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

using namespace std;
using namespace cv;

class MST {
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
                DisjointSet(int n);
                int find(int u);
                void unite(int u, int v);
        };

        float k;

        Mat image;
        Mat segmentedImage;

        vector<Vertice> vertices;
        vector<Edge> edges;

    public:
        MST(string &imagePath, float k);
        void buildGraph();
        int calculateWeight(Vertice v1, Vertice v2);
        Mat segment();
};

#endif // __MST_HPP__