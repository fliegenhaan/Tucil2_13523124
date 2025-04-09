#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <string>
#include "FreeImage.h"

// Struktur Node Quadtree
struct QuadTreeNode {
    int x, y;               // Posisi (koordinat kiri atas)
    int width, height;      // Ukuran blok
    RGBQUAD avgColor;       // Warna rata-rata blok
    bool isLeaf;            // Flag untuk node daun
    
    // Child nodes (untuk non-leaf)
    QuadTreeNode* topLeft;
    QuadTreeNode* topRight;
    QuadTreeNode* bottomLeft;
    QuadTreeNode* bottomRight;
    
    // Constructor
    QuadTreeNode(int _x, int _y, int _width, int _height);
    
    // Destructor
    ~QuadTreeNode();
};

// Fungsi untuk perhitungan warna dan error
RGBQUAD calculateAverageColor(FIBITMAP* image, int x, int y, int width, int height);
double calculateVariance(FIBITMAP* image, int x, int y, int width, int height, RGBQUAD avgColor);
double calculateMAD(FIBITMAP* image, int x, int y, int width, int height, RGBQUAD avgColor);
double calculateMaxDifference(FIBITMAP* image, int x, int y, int width, int height);
double calculateEntropy(FIBITMAP* image, int x, int y, int width, int height);
double calculateSSIM(FIBITMAP* image, int x, int y, int width, int height, RGBQUAD avgColor);
double calculateError(FIBITMAP* image, int x, int y, int width, int height, int method);

// Fungsi untuk pembangunan dan visualisasi Quadtree
QuadTreeNode* buildQuadTree(FIBITMAP* image, int x, int y, int width, int height, 
                            int minBlockSize, double threshold, int method);
void drawQuadTree(FIBITMAP* outputImage, QuadTreeNode* node);
void calculateQuadTreeStats(QuadTreeNode* node, int& nodeCount, int& maxDepth, int currentDepth = 0);
double calculateCompressionPercentage(FIBITMAP* originalImage, int nodeCount);
int getQuadTreeDepth(QuadTreeNode* node);

// Fungsi untuk mencari threshold optimal
double findThresholdForTargetCompression(FIBITMAP* image, int minBlockSize, int method, double targetPercentage);

// Fungsi untuk membuat dan menyimpan GIF menggunakan FreeImage
std::vector<FIBITMAP*> createQuadTreeFrames(FIBITMAP* image, QuadTreeNode* root);

// Fungsi untuk membuat dan menyimpan GIF menggunakan ImageMagick
bool saveGIF(FIBITMAP* originalImage, QuadTreeNode* root, const std::string& outputPath, bool useMagickExe = false);

// Metode utk pengukuran error
const char* getErrorMethodName(int method);

#endif