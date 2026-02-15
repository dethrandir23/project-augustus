#pragma once

#include <string>
#include <vector>
#include <optional>

class FileLoader {
public:
    struct FileData {
        std::string content;
        std::string name;
        std::string extension;
    };

    FileLoader() = delete;

    static std::optional<FileData> readFile(const std::string &path);
    
    static std::optional<std::vector<FileData>> readFiles(const std::vector<std::string> &paths);

    static bool loadFile(const std::string &path);
    static bool loadFiles(const std::vector<std::string> &paths);

    static void clearCache();

    static std::vector<FileData>& getCachedFiles();
    static void setCachedFiles(const std::vector<FileData>& files);

private:
    static std::vector<FileData> cachedFiles;       
};