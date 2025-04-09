#include "quadtree.h"
#include <functional>
#include <cmath>
#include <iostream>
#include <algorithm>

QuadTreeNode* buildQuadTree(FIBITMAP* image, int x, int y, int width, int height, 
                          int minBlockSize, double threshold, int method) {
    QuadTreeNode* node = new QuadTreeNode(x, y, width, height);
    
    node->avgColor = calculateAverageColor(image, x, y, width, height);
    
    double error = calculateError(image, x, y, width, height, method);
    
    // Cek kondisi penghentian:
    // 1. Jika error di bawah threshold, blok tidak perlu dibagi lagi
    // 2. Jika ukuran blok sudah minimum, blok tidak bisa dibagi lagi
    // 3. Jika ukuran blok setelah dibagi akan lebih kecil dari minimum, blok tidak dibagi
    if (error <= threshold || width <= minBlockSize || height <= minBlockSize || 
        width/2 < minBlockSize || height/2 < minBlockSize) {
        node->isLeaf = true;
        return node;
    }
    
    // Divide: Bagi blok menjadi 4 bagian
    int halfWidth = width / 2;
    int halfHeight = height / 2;
    
    // Conquer: Rekursif untuk keempat kuadran
    node->topLeft = buildQuadTree(image, x, y, halfWidth, halfHeight, 
                                minBlockSize, threshold, method);
    
    node->topRight = buildQuadTree(image, x + halfWidth, y, width - halfWidth, 
                                 halfHeight, minBlockSize, threshold, method);
    
    node->bottomLeft = buildQuadTree(image, x, y + halfHeight, halfWidth, 
                                   height - halfHeight, minBlockSize, threshold, method);
    
    node->bottomRight = buildQuadTree(image, x + halfWidth, y + halfHeight, 
                                    width - halfWidth, height - halfHeight, 
                                    minBlockSize, threshold, method);
    
    return node;
}

// Fungsi untuk menggambar Quadtree ke gambar output
void drawQuadTree(FIBITMAP* outputImage, QuadTreeNode* node) {
    if (!node) return;
    
    if (node->isLeaf) {
        for (int j = node->y; j < node->y + node->height; j++) {
            for (int i = node->x; i < node->x + node->width; i++) {
                FreeImage_SetPixelColor(outputImage, i, j, &node->avgColor);
            }
        }
    } else {
        // Rekursif untuk semua anak
        drawQuadTree(outputImage, node->topLeft);
        drawQuadTree(outputImage, node->topRight);
        drawQuadTree(outputImage, node->bottomLeft);
        drawQuadTree(outputImage, node->bottomRight);
    }
}

// Fungsi untuk menghitung statistik Quadtree
void calculateQuadTreeStats(QuadTreeNode* node, int& nodeCount, int& maxDepth, int currentDepth) {
    if (!node) return;
    
    nodeCount++;
    maxDepth = std::max(maxDepth, currentDepth);
    
    if (!node->isLeaf) {
        calculateQuadTreeStats(node->topLeft, nodeCount, maxDepth, currentDepth + 1);
        calculateQuadTreeStats(node->topRight, nodeCount, maxDepth, currentDepth + 1);
        calculateQuadTreeStats(node->bottomLeft, nodeCount, maxDepth, currentDepth + 1);
        calculateQuadTreeStats(node->bottomRight, nodeCount, maxDepth, currentDepth + 1);
    }
}

// Fungsi untuk menghitung persentase kompresi
double calculateCompressionPercentage(FIBITMAP* originalImage, int nodeCount) {
    int width = FreeImage_GetWidth(originalImage);
    int height = FreeImage_GetHeight(originalImage);
    
    // Asumsi gambar RGB dimana setiap pixel membutuhkan 3 bytes
    unsigned long long originalSize = static_cast<unsigned long long>(width) * height * 3;
    
    unsigned long long compressedSize = static_cast<unsigned long long>(nodeCount) * 
                                      (2 * sizeof(int) + 2 * sizeof(int) + sizeof(RGBQUAD) + sizeof(bool));
    
    // persentase kompresi
    double compressionPercentage = (1.0 - static_cast<double>(compressedSize) / originalSize) * 100.0;
    
    return compressionPercentage;
}

// Fungsi untuk mencari threshold optimal untuk target persentase kompresi
double findThresholdForTargetCompression(FIBITMAP* image, int minBlockSize, int method, double targetPercentage) {
    double lowThreshold = 0.0;
    double highThreshold = 100.0; 
    double currentThreshold;
    
    const double TOLERANCE = 0.5; 
    int maxIterations = 15; // Iterasi dibatasi untuk efisiensi
    
    int width = FreeImage_GetWidth(image);
    int height = FreeImage_GetHeight(image);
    
    for (int i = 0; i < maxIterations; i++) {
        currentThreshold = (lowThreshold + highThreshold) / 2.0;
        
        QuadTreeNode* root = buildQuadTree(image, 0, 0, width, height, 
                                          minBlockSize, currentThreshold, method);
        
        int nodeCount = 0, maxDepth = 0;
        calculateQuadTreeStats(root, nodeCount, maxDepth);
        
        double compressionPercentage = calculateCompressionPercentage(image, nodeCount);
        
        delete root;
        
        if (fabs(compressionPercentage - targetPercentage) < TOLERANCE) {
            return currentThreshold;
        }
        
        if (compressionPercentage < targetPercentage) {
            lowThreshold = currentThreshold;
        } else {
            highThreshold = currentThreshold;
        }
    }

    return currentThreshold;
}