#ifndef UTILS_HPP
#define UTILS_HPP

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <string>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

void saveSegmentResult(const Mat& img, const string& imageName, const string& suffix, const string& outDir);

void saveSegmentResultSolucaoB(const Mat& img, const string& imageName, const string& suffix, const string& outDir = "assets/output/B");

#endif