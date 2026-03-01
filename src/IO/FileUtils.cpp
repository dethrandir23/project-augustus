// FileUtils.cpp
#include "FileUtils.h"
#include "lz4/lz4.h"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

/**
 * @namespace FileUtils
 * @brief Utility functions for file system operations, reading, and writing files.
 */
namespace FileUtils {

    std::vector<uint8_t> readFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return data;
    }
    std::string readFileAsString(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return std::string(data.begin(), data.end());
    }

    void writeFile(const std::string& filePath, const std::vector<uint8_t>& data) {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filePath);
        }
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
    }
    void writeFile(const std::string& filePath, const std::string& data) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filePath);
        }
        file << data;
        file.close();
    }

    std::vector<uint8_t> compress(const std::vector<uint8_t>& data) {
        if (data.empty()) {
            return {};
        }
        
        // Orijinal boyutu başta sakla
        uint32_t originalSize = static_cast<uint32_t>(data.size());
        
        // Maksimum sıkıştırılmış boyut
        int maxCompressedSize = LZ4_compressBound(originalSize);
        if (maxCompressedSize <= 0) {
            throw std::runtime_error("LZ4_compressBound failed");
        }
        
        // Çıktı buffer'ı (boyut + sıkıştırılmış veri)
        std::vector<uint8_t> compressed(sizeof(originalSize) + maxCompressedSize);
        
        // Orijinal boyutu başa yaz
        std::memcpy(compressed.data(), &originalSize, sizeof(originalSize));
        
        // Sıkıştır
        int compressedSize = LZ4_compress_default(
            reinterpret_cast<const char*>(data.data()),
            reinterpret_cast<char*>(compressed.data() + sizeof(originalSize)),
            originalSize,
            maxCompressedSize
        );
        
        if (compressedSize <= 0) {
            throw std::runtime_error("LZ4 compression failed");
        }
        
        // Tam boyuta kes (boyut + sıkıştırılmış veri)
        compressed.resize(sizeof(originalSize) + compressedSize);
        return compressed;
    }

    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(uint32_t)) {
            throw std::runtime_error("Invalid compressed data");
        }
        
        uint32_t originalSize;
        std::memcpy(&originalSize, data.data(), sizeof(originalSize));
        
        if (originalSize == 0) {
            return {};
        }
        
        std::vector<uint8_t> decompressed(originalSize);
        
        int decompressedSize = LZ4_decompress_safe(
            reinterpret_cast<const char*>(data.data() + sizeof(originalSize)),
            reinterpret_cast<char*>(decompressed.data()),
            static_cast<int>(data.size() - sizeof(originalSize)),
            originalSize
        );
        
        if (decompressedSize != static_cast<int>(originalSize)) {
            throw std::runtime_error("LZ4 decompression failed");
        }
        
        return decompressed;
    }

    bool exists(const std::string& filePath) {
        return fs::exists(filePath);
    }
    bool isDirectory(const std::string& filePath) {
        return fs::is_directory(filePath);
    }
    bool createDirectories(const std::string& dirPath) {
        return fs::create_directories(dirPath);
    }
    
    std::vector<std::string> listFiles(const std::string& dirPath, bool recursive) {
        std::vector<std::string> files;
        for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().string());
            }
        }
        return files;
    }
    std::string getExtension(const std::string& filePath) {
        return fs::path(filePath).extension().string();
    }
    std::string getFileName(const std::string& filePath) {
        return fs::path(filePath).filename().string();
    }

}