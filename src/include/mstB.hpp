#ifndef MST_HPP
#define MST_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;
using namespace cv;

class MSTB {
public:
    enum class ColorMode {
        RGB,
        GRAYSCALE,
        MEAN_COLOR
    };

    struct Vertice {
        int row;
        int col;
        cv::Vec3b color;
    };

    struct Edge {
        int v1;
        int v2;
        int weight;
    };

    class DisjointSet {
        public:
            vector<int> parent;
            vector<int> size;
            vector<float> internal_diff;

            DisjointSet(int n);

            int find(int u);
            void unite(int u, int v, int weight);
        };

private:
    string imagePath;

    Mat image;
    Mat segmentedImage;

    float Lambda = 40.0f;

    vector<Vertice> vertices;
    vector<Edge> edges;

    vector<Edge> mstEdges;

    void buildMST();

    Mat computeQFZ(float lambda);

    Mat computeSaliencyMap();

    void saveHierarchy();
    
    int calculateWeight(const Vertice& v1, const Vertice& v2);

    void buildGraph();

    Mat renderSegments(
        DisjointSet& ds,
        int width,
        int height,
        ColorMode mode
    );

public:
    MSTB(string& imagePath);

    Mat segment();
};

#endif