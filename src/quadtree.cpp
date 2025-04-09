#include "quadtree.h"
#include <cmath>
#include <functional>
#include <algorithm>

// CTOR dan DTOR
QuadTreeNode::QuadTreeNode(int _x, int _y, int _width, int _height)
    : x(_x), y(_y), width(_width), height(_height), isLeaf(false),
      topLeft(nullptr), topRight(nullptr), bottomLeft(nullptr), bottomRight(nullptr) {
    avgColor.rgbRed = 0;
    avgColor.rgbGreen = 0;
    avgColor.rgbBlue = 0;
    avgColor.rgbReserved = 0;
}

QuadTreeNode::~QuadTreeNode() {
    if (topLeft) delete topLeft;
    if (topRight) delete topRight;
    if (bottomLeft) delete bottomLeft;
    if (bottomRight) delete bottomRight;
}

// Fungsi untuk menghitung rata-rata warna dalam suatu blok
RGBQUAD calculateAverageColor(FIBITMAP* image, int x, int y, int width, int height) {
    RGBQUAD avgColor = {0, 0, 0, 0};
    unsigned long long totalRed = 0, totalGreen = 0, totalBlue = 0;
    int pixelCount = 0;
    
    for (int j = y; j < y + height; j++) {
        for (int i = x; i < x + width; i++) {
            RGBQUAD pixel;
            FreeImage_GetPixelColor(image, i, j, &pixel);
            
            totalRed += pixel.rgbRed;
            totalGreen += pixel.rgbGreen;
            totalBlue += pixel.rgbBlue;
            pixelCount++;
        }
    }
    
    if (pixelCount > 0) {
        avgColor.rgbRed = static_cast<BYTE>(totalRed / pixelCount);
        avgColor.rgbGreen = static_cast<BYTE>(totalGreen / pixelCount);
        avgColor.rgbBlue = static_cast<BYTE>(totalBlue / pixelCount);
    }
    
    return avgColor;
}

// Fungsi untuk menghitung variance
double calculateVariance(FIBITMAP* image, int x, int y, int width, int height, RGBQUAD avgColor) {
    double varR = 0, varG = 0, varB = 0;
    int N = width * height;
    
    for (int j = y; j < y + height; j++) {
        for (int i = x; i < x + width; i++) {
            RGBQUAD pixel;
            FreeImage_GetPixelColor(image, i, j, &pixel);
            
            double diffR = pixel.rgbRed - avgColor.rgbRed;
            double diffG = pixel.rgbGreen - avgColor.rgbGreen;
            double diffB = pixel.rgbBlue - avgColor.rgbBlue;
            
            varR += diffR * diffR;
            varG += diffG * diffG;
            varB += diffB * diffB;
        }
    }
    
    if (N > 0) {
        varR /= N;
        varG /= N;
        varB /= N;
    }
    
    return (varR + varG + varB) / 3.0;
}

// Fungsi untuk menghitung Mean Absolute Deviation (MAD)
double calculateMAD(FIBITMAP* image, int x, int y, int width, int height, RGBQUAD avgColor) {
    double madR = 0, madG = 0, madB = 0;
    int N = width * height;
    
    for (int j = y; j < y + height; j++) {
        for (int i = x; i < x + width; i++) {
            RGBQUAD pixel;
            FreeImage_GetPixelColor(image, i, j, &pixel);
            
            madR += abs(static_cast<int>(pixel.rgbRed) - avgColor.rgbRed);
            madG += abs(static_cast<int>(pixel.rgbGreen) - avgColor.rgbGreen);
            madB += abs(static_cast<int>(pixel.rgbBlue) - avgColor.rgbBlue);
        }
    }
    
    if (N > 0) {
        madR /= N;
        madG /= N;
        madB /= N;
    }
    
    return (madR + madG + madB) / 3.0;
}

// Fungsi untuk menghitung Max Pixel Difference
double calculateMaxDifference(FIBITMAP* image, int x, int y, int width, int height) {
    BYTE minR = 255, minG = 255, minB = 255;
    BYTE maxR = 0, maxG = 0, maxB = 0;
    
    for (int j = y; j < y + height; j++) {
        for (int i = x; i < x + width; i++) {
            RGBQUAD pixel;
            FreeImage_GetPixelColor(image, i, j, &pixel);
            
            minR = std::min(minR, pixel.rgbRed);
            minG = std::min(minG, pixel.rgbGreen);
            minB = std::min(minB, pixel.rgbBlue);
            
            maxR = std::max(maxR, pixel.rgbRed);
            maxG = std::max(maxG, pixel.rgbGreen);
            maxB = std::max(maxB, pixel.rgbBlue);
        }
    }
    
    double diffR = maxR - minR;
    double diffG = maxG - minG;
    double diffB = maxB - minB;
    
    return (diffR + diffG + diffB) / 3.0;
}

// Fungsi untuk menghitung Entropy
double calculateEntropy(FIBITMAP* image, int x, int y, int width, int height) {
    int histR[256] = {0}, histG[256] = {0}, histB[256] = {0};
    int N = width * height;
    
    for (int j = y; j < y + height; j++) {
        for (int i = x; i < x + width; i++) {
            RGBQUAD pixel;
            FreeImage_GetPixelColor(image, i, j, &pixel);
            
            histR[pixel.rgbRed]++;
            histG[pixel.rgbGreen]++;
            histB[pixel.rgbBlue]++;
        }
    }
    
    // Hitung entropy
    double entropyR = 0, entropyG = 0, entropyB = 0;
    
    for (int i = 0; i < 256; i++) {
        if (histR[i] > 0) {
            double pR = static_cast<double>(histR[i]) / N;
            entropyR -= pR * log2(pR);
        }
        
        if (histG[i] > 0) {
            double pG = static_cast<double>(histG[i]) / N;
            entropyG -= pG * log2(pG);
        }
        
        if (histB[i] > 0) {
            double pB = static_cast<double>(histB[i]) / N;
            entropyB -= pB * log2(pB);
        }
    }
    
    return (entropyR + entropyG + entropyB) / 3.0;
}

// Fungsi untuk menghitung SSIM 
double calculateSSIM(FIBITMAP* image, int x, int y, int width, int height, RGBQUAD avgColor) {
    // Konstanta untuk stabilitas
    const double C1 = 6.5025;   // (0.01 * 255)²
    const double C2 = 58.5225;  // (0.03 * 255)²
    
    // Bobot untuk setiap channel (defaultnya sama)
    const double wR = 0.33333, wG = 0.33333, wB = 0.33334;
    
    std::vector<double> sourceR, sourceG, sourceB;
    std::vector<double> targetR, targetG, targetB;
    
    // Isi gambar sumber dari blok asli
    for (int j = y; j < y + height; j++) {
        for (int i = x; i < x + width; i++) {
            RGBQUAD pixel;
            FreeImage_GetPixelColor(image, i, j, &pixel);
            
            sourceR.push_back(static_cast<double>(pixel.rgbRed));
            sourceG.push_back(static_cast<double>(pixel.rgbGreen));
            sourceB.push_back(static_cast<double>(pixel.rgbBlue));
            
            // Gambar target adalah blok dengan warna rata-rata yang sama
            targetR.push_back(static_cast<double>(avgColor.rgbRed));
            targetG.push_back(static_cast<double>(avgColor.rgbGreen));
            targetB.push_back(static_cast<double>(avgColor.rgbBlue));
        }
    }
    
    // Hitung statistik untuk setiap channel warna
    // Sumber (gambar asli)
    double muX_R = 0, muX_G = 0, muX_B = 0;
    double sigmaX2_R = 0, sigmaX2_G = 0, sigmaX2_B = 0;
    
    // Target (gambar dengan warna konstan rata-rata)
    double muY_R = avgColor.rgbRed;
    double muY_G = avgColor.rgbGreen;
    double muY_B = avgColor.rgbBlue;
    double sigmaY2_R = 0, sigmaY2_G = 0, sigmaY2_B = 0;  
    
    // Kovariansi
    double sigmaXY_R = 0, sigmaXY_G = 0, sigmaXY_B = 0;
    
    // Hitung mean
    int n = sourceR.size();
    for (int i = 0; i < n; i++) {
        muX_R += sourceR[i];
        muX_G += sourceG[i];
        muX_B += sourceB[i];
    }
    
    muX_R /= n;
    muX_G /= n;
    muX_B /= n;
    
    // Hitung variansi dan kovariansi
    for (int i = 0; i < n; i++) {
        double diffX_R = sourceR[i] - muX_R;
        double diffX_G = sourceG[i] - muX_G;
        double diffX_B = sourceB[i] - muX_B;
        
        sigmaX2_R += diffX_R * diffX_R;
        sigmaX2_G += diffX_G * diffX_G;
        sigmaX2_B += diffX_B * diffX_B;
        
        sigmaXY_R += diffX_R * (targetR[i] - muY_R);
        sigmaXY_G += diffX_G * (targetG[i] - muY_G);
        sigmaXY_B += diffX_B * (targetB[i] - muY_B);
    }
    
    sigmaX2_R /= n;
    sigmaX2_G /= n;
    sigmaX2_B /= n;
    
    sigmaXY_R /= n;
    sigmaXY_G /= n;
    sigmaXY_B /= n;
    
    // Hitung SSIM untuk setiap channel
    double ssim_R = ((2 * muX_R * muY_R + C1) * (2 * sigmaXY_R + C2)) / 
                   ((muX_R * muX_R + muY_R * muY_R + C1) * (sigmaX2_R + sigmaY2_R + C2));
                   
    double ssim_G = ((2 * muX_G * muY_G + C1) * (2 * sigmaXY_G + C2)) / 
                   ((muX_G * muX_G + muY_G * muY_G + C1) * (sigmaX2_G + sigmaY2_G + C2));
                   
    double ssim_B = ((2 * muX_B * muY_B + C1) * (2 * sigmaXY_B + C2)) / 
                   ((muX_B * muX_B + muY_B * muY_B + C1) * (sigmaX2_B + sigmaY2_B + C2));
    
    double ssim = wR * ssim_R + wG * ssim_G + wB * ssim_B;
    
    // SSIM berkisar dari 0 hingga 1, dengan 1 menunjukkan kesamaan sempurna
    // Untuk kompresi quadtree, nilai error yang tinggi = kurang mirip
    const double SCALE_FACTOR = 10000.0;
    
    // Jika ssim mendekati 1 (sangat mirip), maka error akan mendekati 0
    double error = (1.0 - ssim) * SCALE_FACTOR;
    
    return error;
}

// Fungsi untuk memilih metode error dan memanggil fungsinya
double calculateError(FIBITMAP* image, int x, int y, int width, int height, int method) {
    RGBQUAD avgColor = calculateAverageColor(image, x, y, width, height);
    
    switch (method) {
        case 1: 
            return calculateVariance(image, x, y, width, height, avgColor);
        case 2: 
            return calculateMAD(image, x, y, width, height, avgColor);
        case 3:
            return calculateMaxDifference(image, x, y, width, height);
        case 4:
            return calculateEntropy(image, x, y, width, height);
        case 5:
            return calculateSSIM(image, x, y, width, height, avgColor);
        default:
            return calculateVariance(image, x, y, width, height, avgColor);
    }
}

// Fungsi untuk mendapatkan nama metode pengukuran error
const char* getErrorMethodName(int method) {
    switch (method) {
        case 1: return "Variance";
        case 2: return "MAD";
        case 3: return "Max Pixel Difference";
        case 4: return "Entropy";
        case 5: return "SSIM";
        default: return "Unknown";
    }
}