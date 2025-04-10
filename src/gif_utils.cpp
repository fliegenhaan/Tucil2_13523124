#include "quadtree.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>
#include <sys/stat.h>
#include <functional>

bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

// Fungsi untuk membuat frame dari quadtree untuk kedalaman tertentu
FIBITMAP* createFrameAtDepth(QuadTreeNode* root, int width, int height, int maxDepth) {
    FIBITMAP* frameBitmap = FreeImage_Allocate(width, height, 24);
    if (!frameBitmap) {
        std::cerr << "Error: Gagal mengalokasikan memori untuk frame!" << std::endl;
        return nullptr;
    }

    std::function<void(QuadTreeNode*, int)> drawNodesUpToDepth = 
        [&frameBitmap, &drawNodesUpToDepth, maxDepth](QuadTreeNode* node, int depth) {
        if (!node) return;
        
        if (node->isLeaf || depth >= maxDepth) {
            for (int j = node->y; j < node->y + node->height; j++) {
                for (int i = node->x; i < node->x + node->width; i++) {
                    FreeImage_SetPixelColor(frameBitmap, i, j, &node->avgColor);
                }
            }
        } else {
            drawNodesUpToDepth(node->topLeft, depth + 1);
            drawNodesUpToDepth(node->topRight, depth + 1);
            drawNodesUpToDepth(node->bottomLeft, depth + 1);
            drawNodesUpToDepth(node->bottomRight, depth + 1);
        }
    };
    
    drawNodesUpToDepth(root, 0);
    
    return frameBitmap;
}

// Fungsi untuk mendapatkan kedalaman maksimum quadtree
int getQuadTreeDepth(QuadTreeNode* node) {
    if (!node) return 0;
    if (node->isLeaf) return 0;
    
    int topLeftDepth = getQuadTreeDepth(node->topLeft);
    int topRightDepth = getQuadTreeDepth(node->topRight);
    int bottomLeftDepth = getQuadTreeDepth(node->bottomLeft);
    int bottomRightDepth = getQuadTreeDepth(node->bottomRight);
    
    int maxChildDepth = std::max(
        std::max(topLeftDepth, topRightDepth),
        std::max(bottomLeftDepth, bottomRightDepth)
    );
    
    return maxChildDepth + 1;
}

// Fungsi untuk menyimpan GIF
bool saveGIF(FIBITMAP* originalImage, QuadTreeNode* root, const std::string& outputPath, bool useMagickExe) {
    int width = FreeImage_GetWidth(originalImage);
    int height = FreeImage_GetHeight(originalImage);
    
    int maxDepth = getQuadTreeDepth(root);
    std::cout << "Kedalaman pohon quadtree: " << maxDepth << std::endl;
    
    std::vector<std::string> frameFilenames;
    
    // buat nampilin frame original tapi di-disable biar ga ikut masuk frame originalnya
    // std::string originalFrame = "frame_original.png";
    // frameFilenames.push_back(originalFrame);
    // FreeImage_Save(FIF_PNG, originalImage, originalFrame.c_str(), 0);
    
    for (int depth = 0; depth <= maxDepth; depth++) {
        bool hasNonLeafAtThisDepth = false;

        std::function<void(QuadTreeNode*, int)> checkNode =
            [&](QuadTreeNode* node, int currentDepth) {
                if (!node || node->isLeaf) return;
                if (currentDepth == depth) hasNonLeafAtThisDepth = true;
                else {
                    checkNode(node->topLeft, currentDepth + 1);
                    checkNode(node->topRight, currentDepth + 1);
                    checkNode(node->bottomLeft, currentDepth + 1);
                    checkNode(node->bottomRight, currentDepth + 1);
                }
            };

        checkNode(root, 0);
        if (!hasNonLeafAtThisDepth) {
            std::cout << "Tidak ada node non-leaf pada depth " << depth << ", menghentikan frame di sini." << std::endl;
            break;
        }

        std::cout << "Membuat frame untuk kedalaman " << depth << "..." << std::endl;
        FIBITMAP* frameBitmap = createFrameAtDepth(root, width, height, depth);
        if (!frameBitmap) {
            std::cerr << "Error: Gagal membuat frame untuk kedalaman " << depth << std::endl;
            continue;
        }
        
        std::string frameFile = "frame_" + std::to_string(depth) + ".png";
        frameFilenames.push_back(frameFile);
        
        if (!FreeImage_Save(FIF_PNG, frameBitmap, frameFile.c_str(), 0)) {
            std::cerr << "Error: Gagal menyimpan frame " << depth << std::endl;
        }
        
        FreeImage_Unload(frameBitmap);
    }
    
    std::ostringstream cmd;
    
    if (useMagickExe) {
        cmd << "magick.exe -delay 50 -loop 0";
    } else {
        cmd << "convert -delay 50 -loop 0";
    }
    
    for (const auto& frame : frameFilenames) {
        if (fileExists(frame)) {
            cmd << " " << frame;
        } else {
            std::cerr << "Peringatan: File frame " << frame << " tidak ditemukan." << std::endl;
        }
    }
    
    cmd << " " << outputPath;
    
    std::cout << "Menjalankan perintah: " << cmd.str() << std::endl;
    int ret = system(cmd.str().c_str());
    
    if (ret != 0) {
        std::cerr << "Error: Gagal membuat GIF menggunakan ImageMagick. Kode error: " << ret << std::endl;
    } else {
        std::cout << "GIF berhasil dibuat!" << std::endl;
    }
    
    for (const auto& frame : frameFilenames) {
        if (fileExists(frame)) {
            if (std::remove(frame.c_str()) != 0) {
                std::cerr << "Error: Gagal menghapus frame sementara " << frame << std::endl;
            }
        }
    }
    
    return (ret == 0);
}