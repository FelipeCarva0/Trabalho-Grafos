#ifndef MST_HPP
#define MST_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <unordered_map>

class MST {
public:
    enum class ColorMode {
        RGB,
        GRAYSCALE
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
        std::vector<int> parent;
        std::vector<int> size;
        std::vector<float> internal_diff;

        DisjointSet(int n);

        int find(int u);
        void unite(int u, int v, int weight);
    };

private:
    std::string imagePath;
    float k;

    cv::Mat image;
    cv::Mat segmentedImage;

    std::vector<Vertice> vertices;
    std::vector<Edge> edges;

    std::vector<Edge> mstEdges;

    void buildMST();

    cv::Mat computeQFZ(float lambda);

    cv::Mat computeSaliencyMap();

    void saveHierarchy();
    
    int calculateWeight(const Vertice& v1, const Vertice& v2);

    void buildGraph();

    cv::Mat renderSegments(
        DisjointSet& ds,
        int width,
        int height,
        ColorMode mode
    );

    cv::Mat renderSegmentsByMeanColor(
        DisjointSet& ds,
        int width,
        int height
    );

public:
    MST(std::string& imagePath, float k);

    cv::Mat segment();
};

#endif