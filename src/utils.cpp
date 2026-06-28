#include <iostream>
#include "include/mst.hpp"
#include "include/mstB.hpp"
#include "include/ift.hpp"
#include "include/utils.hpp"

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

void saveSegmentResult(const Mat& img, const string& imageName, const string& suffix, const string& outDir){
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