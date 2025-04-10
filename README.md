# Tugas Kecil 2 IF2211 Strategi Algoritma - 13523124
# Kompresi Gambar dengan Metode Quadtree

## Deskripsi Singkat

Program ini mengimplementasikan metode kompresi gambar berbasis Quadtree dengan pendekatan Divide and Conquer. Program menerima gambar input, kemudian menerapkan algoritma divide and conquer untuk membagi gambar menjadi blok-blok berbentuk persegi yang lebih kecil berdasarkan keseragaman warna atau intensitas piksel. 

Proses pembagian dilakukan secara rekursif dan akan berhenti ketika blok memenuhi kriteria homogenitas (error berada di bawah threshold) atau mencapai ukuran minimum yang ditentukan. Hasilnya adalah representasi gambar yang terkompresi dengan mempertahankan detail pada area yang kompleks dan melakukan pengelompokan pada area yang homogen.

## Fitur

- Berbagai metode perhitungan error:
  - Variance
  - Mean Absolute Deviation (MAD)
  - Max Pixel Difference
  - Entropy
  - Structural Similarity Index (SSIM) [Bonus]
- Konfigurasi parameter kompresi:
  - Ambang batas (threshold)
  - Ukuran blok minimum
  - Target persentase kompresi [Bonus]
- Statistik kompresi:
  - Waktu eksekusi
  - Ukuran gambar sebelum dan sesudah kompresi
  - Persentase kompresi
  - Kedalaman pohon
  - Jumlah simpul (node)
- Visualisasi proses pembentukan Quadtree dalam bentuk GIF [Bonus]

## Requirement dan Instalasi

### Requirement:
- C++ compiler yang mendukung C++11 atau lebih tinggi (seperti GCC, MinGW, atau MSVC)
- Library FreeImage (disertakan dalam repository)
- ImageMagick (opsional, untuk membuat GIF)

### Instalasi:
1. Clone repository ini:
   ```bash
   git clone https://github.com/yourusername/Tucil2_13523124.git
   cd Tucil2_13523124
   ```
2. Opsional - Install ImageMagick untuk fitur pembuatan GIF:
   Windows: Download dari ImageMagick Website (https://imagemagick.org/script/download.php)
   Ubuntu/Debian: sudo apt-get install imagemagick
   Mac: brew install imagemagick
3. Install FreeImage untuk pemrosesan gambar:
   Untuk Windows:
   - https://freeimage.sourceforge.io/ (sesuaikan yang Win32/Win64)
   - Ekstrak dari zip lalu buka folder Dist/(Win32/Win64)
   - Pindahkan file FreeImage.h dan FreeImage.lib ke lib/FreeImage/
   - Pindahkan file FreeImage.dll ke bin/
  -  Untuk Linux:
   ```bash
   sudo apt install libfreeimage-dev
   ```
## Cara menjalankan Program
### Cara Kompilasi
1. Kompilasi program dengan Windows (dengan MinGW) dan Linux
```bash
g++ -o bin/quadtree_compression src/*.cpp -I lib/FreeImage -L lib/FreeImage -lfreeimage
```
2. Jalankan program executable
```bash
./bin/quadtree_compression
```
3. Ikuti petunjuk yang muncul:
- Masukkan alamat absolut gambar yang akan dikompresi (misal dalam folder test, inputnya: test/ori.png)
- Pilih metode perhitungan error (1-5)
- Masukkan nilai threshold
- Masukkan ukuran blok minimum
- Masukkan target persentase kompresi (0 untuk menonaktifkan)
- Masukkan alamat absolut untuk menyimpan gambar hasil
- Masukkan alamat absolut untuk menyimpan GIF (opsional)
## Author
Nama : Muhammad Raihaan Perdana
NIM : 13523124
Kelas : K03
