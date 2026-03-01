// FileUtils.h

#pragma once
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>

/**
 * @namespace FileUtils
 * @brief Utility functions for file system operations, reading, and writing files.
 */
namespace FileUtils {

    std::vector<uint8_t> readFile(const std::string& filePath);
    std::string readFileAsString(const std::string& filePath);

    void writeFile(const std::string& filePath, const std::vector<uint8_t>& data);
    void writeFile(const std::string& filePath, const std::string& data);

    std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);

    bool exists(const std::string& filePath);
    bool isDirectory(const std::string& filePath);
    bool createDirectories(const std::string& dirPath);
    
    std::vector<std::string> listFiles(const std::string& dirPath, bool recursive = false);
    std::string getExtension(const std::string& filePath);
    std::string getFileName(const std::string& filePath);

}