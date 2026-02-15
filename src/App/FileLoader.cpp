#include "FileLoader.h"
#include <filesystem>
#include <fstream>
#include "../DevTools/Console.h"

std::vector<FileLoader::FileData> FileLoader::cachedFiles;

std::optional<FileLoader::FileData> FileLoader::readFile(const std::string &path) {
    namespace fs = std::filesystem;

    if (!fs::exists(path)) {
        Console::log("File not found: " + path, LogType::ERROR);
        return std::nullopt;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        Console::log("Failed to open file: " + path, LogType::ERROR);
        return std::nullopt;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    std::string ext = fs::path(path).extension().string();
    std::string name = fs::path(path).stem().string();


    return FileData{ content, name, ext };
}

std::optional<std::vector<FileLoader::FileData>> FileLoader::readFiles(const std::vector<std::string> &paths) {
    std::vector<FileData> loadedFiles;
    loadedFiles.reserve(paths.size());

    for (const auto &path : paths) {
        auto result = readFile(path);
        
        if (result.has_value()) {
            loadedFiles.push_back(result.value());
        } else {
            Console::log("Batch read failed at: " + path, LogType::ERROR);
            return std::nullopt;
        }
    }
    return loadedFiles;
}

bool FileLoader::loadFile(const std::string &path) {
    auto result = readFile(path);
    
    if (result.has_value()) {
        cachedFiles.push_back(result.value());
        return true;
    }
    
    return false;
}

bool FileLoader::loadFiles(const std::vector<std::string> &paths) {
    auto result = readFiles(paths);

    if (result.has_value()) {
        cachedFiles.insert(cachedFiles.end(), result->begin(), result->end());
        return true;
    }

    return false;
}

void FileLoader::clearCache() {
    cachedFiles.clear();
}

std::vector<FileLoader::FileData>& FileLoader::getCachedFiles() {
    return cachedFiles;
}

void FileLoader::setCachedFiles(const std::vector<FileData>& files) {
    cachedFiles = files;
}