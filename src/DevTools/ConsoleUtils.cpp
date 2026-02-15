#include "ConsoleUtils.h"
#include "Console.h"
#include "Game/IdUtils.h"
#include "uuid/uuid.h"
#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>

// Yardımcı fonksiyon (Helper)
std::chrono::system_clock::time_point stringToTimePoint(const std::string& timeString) {
    std::tm tm = {};
    std::stringstream ss(timeString);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) throw std::runtime_error("Tarih formati hatali!");
    std::time_t tt = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(tt);
}

void ConsoleUtils::printConsole() {
    for (const auto &l : Console::getLogs()) {
        std::cout << l << std::endl;
    }
}

void ConsoleUtils::printConsole(bool timestamp, bool type) {
    for (const auto &l : Console::getLogs()) {
        std::string str;
        if (timestamp) str += "[" + l.timestamp + "] ";
        if (type)      str += "[" + logTypeToStr(l.logType) + "] ";
        str += l.message;
        std::cout << str << std::endl;
    }
}

nlohmann::json logToJson(const Log& log) {
    return nlohmann::json{
        {"timestamp", log.timestamp},
        {"type", logTypeToStr(log.logType)},
        {"message", log.message}
    };
}

bool ConsoleUtils::saveConsoleToFile(const std::string& filename, ExportFormat format) {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) return false;

        auto logs = Console::getLogs();

        switch (format) {
            case ExportFormat::TXT:
                for (const auto &l : logs) {
                    file << l << "\n"; // Log struct'ındaki operator<< kullanılıyor
                }
                break;

            case ExportFormat::JSON: {
                nlohmann::json j = nlohmann::json::array();
                for (const auto &l : logs) {
                    j.push_back(logToJson(l));
                }
                file << j.dump(4); // 4 boşluk girintili (pretty print)
                break;
            }

            case ExportFormat::CSV:
                file << "Timestamp,Type,Message\n"; // Header
                for (const auto &l : logs) {
                    // CSV'de virgül karmaşasını önlemek için mesajı tırnak içine alıyoruz
                    file << l.timestamp << "," 
                         << logTypeToStr(l.logType) << ",\"" 
                         << l.message << "\"\n";
                }
                break;
        }

        file.close();
        return true;
    } catch(const std::exception &e) {
        std::cerr << "Save error: " << e.what() << '\n';
        return false;
    }
}

void ConsoleUtils::saveConsoleToFile(const fs::path &filename, ExportFormat format) {
    saveConsoleToFile(filename.string(), format);
}

fs::path ConsoleUtils::saveConsoleToTempFile(ExportFormat format) {
    std::string ext = ".log";
    if (format == ExportFormat::JSON) ext = ".json";
    else if (format == ExportFormat::CSV) ext = ".csv";

    auto temp_file = fs::temp_directory_path() / (uuids::to_string(IdUtils::generateUuid()) + "_console" + ext);
    
    if (saveConsoleToFile(temp_file.string(), format)) {
        return temp_file;
    }
    return "";
}

bool ConsoleUtils::loadConsoleFromFile(const std::string& filename) {
    fs::path filePath(filename);
    std::string ext = filePath.extension().string();
    
    // Küçük harfe çevirelim ki .JSON veya .csv gibi durumlarda patlamasın
    for (auto &c : ext) c = std::tolower(c);

    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            Console::log("Dosya acilamadi: " + filename, LogType::ERROR);
            return false;
        }

        Console::clearLogs();

        if (ext == ".json") {
            nlohmann::json j;
            file >> j;
            for (const auto& item : j) {
                // JSON'dan verileri çekip Log objesi oluşturuyoruz
                std::string ts = item.value("timestamp", "");
                LogType type = strToLogType(item.value("type", "INFO"));
                std::string msg = item.value("message", "");
                Console::log(Log(ts, type, msg));
            }
        } 
        else if (ext == ".csv") {
            std::string line;
            bool isHeader = true;
            while (std::getline(file, line)) {
                if (isHeader) { isHeader = false; continue; } // İlk satırı (Header) atla
                if (line.empty()) continue;

                // Basit bir CSV ayırıcı (Virgül ve tırnak kontrolü)
                std::stringstream ss(line);
                std::string ts, typeStr, msg;
                
                std::getline(ss, ts, ',');
                std::getline(ss, typeStr, ',');
                std::getline(ss, msg); // Kalan her şey mesajdır

                // Kaydederken tırnak eklemiştik, yüklerken onları temizleyelim
                if (msg.front() == '"' && msg.back() == '"') {
                    msg = msg.substr(1, msg.length() - 2);
                }

                Console::log(Log(ts, strToLogType(typeStr), msg));
            }
        }
        else if (ext == ".txt" || ext == ".log") {
            static auto logRegex = std::regex(R"(^\[(.*?)\]\s+\[(.*?)\]\s+(.*)$)");
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty()) continue;
                std::smatch match;
                if (std::regex_search(line, match, logRegex)) {
                    Console::log(Log(match[1].str(), strToLogType(match[2].str()), match[3].str()));
                }
            }
        } 
        else {
            Console::log("Unsupported format: " + ext, LogType::ERROR);
            return false;
        }

        return true;
    } catch (const std::exception &e) {
        Console::log("Yükleme hatası: " + std::string(e.what()), LogType::ERROR);
        return false;
    }
}

bool ConsoleUtils::loadConsoleFromFile(const fs::path &filename) {
    return loadConsoleFromFile(filename.string());
}

std::vector<Log> ConsoleUtils::selectLogsByType(LogType type) {
    std::vector<Log> filtered;
    for (const auto &log : Console::getLogs()) {
        if (log.logType == type) filtered.push_back(log);
    }
    return filtered;
}

std::vector<Log> ConsoleUtils::selectLogsByTimestamp(const std::chrono::system_clock::time_point &firstTime, const std::chrono::system_clock::time_point &lastTime) {
    std::vector<Log> filtered;
    for (const auto &log : Console::getLogs()) {
        auto t = stringToTimePoint(log.timestamp);
        if (t >= firstTime && t <= lastTime) filtered.push_back(log);
    }
    return filtered;
}

std::vector<Log> ConsoleUtils::selectLogsByMessage(const std::string& message) {
    std::vector<Log> filtered;
    for (const auto &log : Console::getLogs()) {
        if (log.message == message) filtered.push_back(log);
    }
    return filtered;
}