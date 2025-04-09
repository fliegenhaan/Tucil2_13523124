#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>
#include "quadtree.h"

using namespace std;

int main() {
    FreeImage_Initialise();
    
    FreeImage_SetOutputMessage([](FREE_IMAGE_FORMAT fif, const char *message) {
        cout << "FreeImage Error";
        if (fif != FIF_UNKNOWN) {
            cout << " (" << FreeImage_GetFormatFromFIF(fif) << ")";
        }
        cout << ": " << message << endl;
    });
    
    string inputPath, outputPath, gifPath;
    int method;
    double threshold;
    int minBlockSize;
    double targetCompression;
    
    cout << "===== KOMPRESI GAMBAR DENGAN METODE QUADTREE =====" << endl;
    cout << "Masukkan alamat absolut gambar yang akan dikompresi: ";
    getline(cin, inputPath);
    
    cout << "===== Metode Perhitungan Error =====" << endl;
    cout << "1. Variance" << endl;
    cout << "2. MAD" << endl;
    cout << "3. Max Pixel Difference" << endl;
    cout << "4. SSIM (Bonus)" << endl;
    cout << "Masukkan metode perhitungan error: ";
    cin >> method;
    
    cout << "Masukkan ambang batas (threshold): ";
    cin >> threshold;
    
    cout << "Masukkan ukuran blok minimum: ";
    cin >> minBlockSize;
    
    cout << "Masukkan target persentase kompresi (0 untuk menonaktifkan): ";
    cin >> targetCompression;
    
    cin.ignore();
    
    cout << "Masukkan alamat absolut gambar hasil kompresi: ";
    getline(cin, outputPath);
    
    cout << "Masukkan alamat absolut gif (kosongkan untuk melewati): ";
    getline(cin, gifPath);
    
    if (method < 0 || method > 4) {
        cout << "Metode perhitungan error tidak valid! Menggunakan metode default (Variance)." << endl;
        method = 0;
    }
    
    if (minBlockSize <= 0) {
        cout << "Ukuran blok minimum tidak valid! Menggunakan nilai default (4)." << endl;
        minBlockSize = 4;
    }
    
    // Deteksi format gambar input
    FREE_IMAGE_FORMAT inputFormat = FreeImage_GetFileType(inputPath.c_str());
    if (inputFormat == FIF_UNKNOWN) {
        inputFormat = FreeImage_GetFIFFromFilename(inputPath.c_str());
    }
    
    if (inputFormat == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(inputFormat)) {
        cout << "Format gambar input tidak didukung atau file tidak ditemukan!" << endl;
        FreeImage_DeInitialise();
        return 1;
    }
    
    // Load gambar input
    FIBITMAP* originalImage = FreeImage_Load(inputFormat, inputPath.c_str());
    if (!originalImage) {
        cout << "Gagal memuat gambar input!" << endl;
        FreeImage_DeInitialise();
        return 1;
    }
    
    FIBITMAP* image = FreeImage_ConvertTo24Bits(originalImage);
    FreeImage_Unload(originalImage);
    
    // Dapatkan dimensi gambar
    int width = FreeImage_GetWidth(image);
    int height = FreeImage_GetHeight(image);
    
    cout << "\nMemproses gambar " << width << "x" << height << " pixel..." << endl;
    
    auto startTime = chrono::high_resolution_clock::now();
    
    // Jika target persentase kompresi diaktifkan, temukan threshold optimal
    if (targetCompression > 0) {
        cout << "Mencari threshold optimal untuk target persentase kompresi " << targetCompression << "%..." << endl;
        threshold = findThresholdForTargetCompression(image, minBlockSize, method, targetCompression);
        cout << "Menggunakan threshold optimal: " << threshold << endl;
    }
    
    cout << "Membangun quadtree..." << endl;
    QuadTreeNode* root = buildQuadTree(image, 0, 0, width, height, minBlockSize, threshold, method);
    
    // Buat gambar output
    FIBITMAP* outputImage = FreeImage_Allocate(width, height, 24);
    if (!outputImage) {
        cout << "Gagal membuat gambar output!" << endl;
        FreeImage_Unload(image);
        delete root;
        FreeImage_DeInitialise();
        return 1;
    }
    
    cout << "Menggambar hasil kompresi..." << endl;
    drawQuadTree(outputImage, root);
    
    // Hitung statistik quadtree
    int nodeCount = 0;
    int maxDepth = 0;
    calculateQuadTreeStats(root, nodeCount, maxDepth);
    
    // Hitung ukuran dan persentase kompresi
    DWORD originalSize = static_cast<DWORD>(width) * height * 3; // 3 bytes per pixel untuk RGB
    DWORD compressedSize = nodeCount * (sizeof(int) * 4 + sizeof(RGBQUAD) + sizeof(bool));
    double compressionPercentage = calculateCompressionPercentage(image, nodeCount);
    
    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
    
    // Simpan gambar output
    FREE_IMAGE_FORMAT outputFormat = FreeImage_GetFIFFromFilename(outputPath.c_str());
    if (outputFormat == FIF_UNKNOWN) {
        outputFormat = FIF_PNG; 
    }
    
    if (!FreeImage_FIFSupportsWriting(outputFormat)) {
        cout << "Format output tidak didukung untuk penyimpanan. Menggunakan PNG sebagai gantinya." << endl;
        outputFormat = FIF_PNG;
    }
    
    bool saveSuccess = FreeImage_Save(outputFormat, outputImage, outputPath.c_str());
    if (!saveSuccess) {
        cout << "Gagal menyimpan gambar output!" << endl;
    } else {
        cout << "Gambar hasil kompresi berhasil disimpan ke: " << outputPath << endl;
    }
    
    // Tampilan statistik
    cout << "\n===== STATISTIK KOMPRESI =====" << endl;
    cout << "Waktu eksekusi: " << duration << " ms" << endl;
    cout << "Ukuran gambar sebelum: " << originalSize << " bytes" << endl;
    cout << "Ukuran gambar setelah: " << compressedSize << " bytes" << endl;
    cout << "Persentase kompresi: " << compressionPercentage << "%" << endl;
    cout << "Kedalaman pohon: " << maxDepth << endl;
    cout << "Banyak simpul pada pohon: " << nodeCount << endl;
    
    // Fungsi membuat GIF
    if (!gifPath.empty()) {
        cout << "\nMemulai proses pembuatan GIF..." << endl;

        string ext = gifPath.substr(gifPath.find_last_of(".") + 1);
        if (ext != "gif" && ext != "GIF") {
            cout << "Menambahkan ekstensi .gif ke nama file" << endl;
            gifPath += ".gif";
        }

        std::replace(gifPath.begin(), gifPath.end(), '\\', '/');

        // Untuk Windows
        #ifdef _WIN32
        if (system("where magick > nul 2>&1") == 0) {
            cout << "Menggunakan ImageMagick versi 7+..." << endl;
            if (saveGIF(image, root, gifPath, true)) {
                cout << "GIF berhasil disimpan ke: " << gifPath << endl;
            } else {
                cout << "Gagal menyimpan GIF dengan ImageMagick 7+." << endl;
            }
        } else if (system("where convert > nul 2>&1") == 0) {
            cout << "Menggunakan ImageMagick legacy command..." << endl;
            if (saveGIF(image, root, gifPath, false)) {
                cout << "GIF berhasil disimpan ke: " << gifPath << endl;
            } else {
                cout << "Gagal menyimpan GIF dengan ImageMagick legacy command." << endl;
            }
        } else {
            cout << "ImageMagick tidak ditemukan. Pastikan ImageMagick terinstal pada sistem Anda." << endl;
            cout << "Anda dapat mengunduh ImageMagick dari: https://imagemagick.org/script/download.php" << endl;
            cout << "Dan pastikan untuk mencentang 'Add application directory to your system path' saat instalasi." << endl;
        }
        #else
        // Pendekatan standard untuk Linux/Mac
        if (saveGIF(image, root, gifPath, false)) {
            cout << "GIF berhasil disimpan ke: " << gifPath << endl;
        } else {
            cout << "Gagal menyimpan GIF. Pastikan ImageMagick terinstal pada sistem Anda." << endl;
            cout << "Anda dapat mengunduh ImageMagick dari: https://imagemagick.org/script/download.php" << endl;
        }
        #endif
    }
    
    FreeImage_Unload(image);
    FreeImage_Unload(outputImage);
    delete root;
    
    FreeImage_DeInitialise();
    
    return 0;
}