// #include "../Api/NativeApi.h"
// #include "DevTools/Console.h"
// #include "ModLoader.h"
// #include "DevTools/ConsoleUtils.h"
// #include <filesystem>
// #include <iostream>
// #include <thread>
// #include <chrono>

// namespace fs = std::filesystem;

// int main(int argc, char **argv) {
//     // --- 1. ARGUMENT PARSING ---
//     if (argc != 2) {
//         std::cerr << "Kullanim: " << argv[0] << " <load_order.json>\n";
//         return 1;
//     }

//     auto path = fs::path(argv[1]);
//     if (!fs::exists(path)) {
//         std::cerr << "Dosya bulunamadi: " << argv[1] << "\n";
//         return 1;
//     }

//     std::cout << "[INFO] Modlar onbellege aliniyor...\n";
//     if (!ModLoader::loadMods(path)) {
//         std::cerr << "[ERROR] Modlar onbellege alinamadi.\n";
//         return 1;
//     }

//     std::cout << "[INFO] " << ModLoader::getModCount() << " mod basariyla onbellege alindi.\n";

//     // --- 2. ENGINE INIT & DATA LOADING ---
//     GameApi::initEngine();

//     if (!ModLoader::insertDataIntoEngine(GameApi::loadGameFiles)) {
//         std::cerr << "[ERROR] Mod verileri motora yuklenemedi.\n";
//         return 1;
//     }

//     std::cout << "[INFO] Modlar motora entegre edildi.\n";
//     std::cout << "[INFO] TradeFall Motoru baslatiliyor...\n";

//     // --- 3. SENARYO VE OYUNCU KURULUMU ---
//     if (!GameApi::startScenario("debug_scenario")) {
//         std::cerr << "[ERROR] Senaryo yuklenemedi. Cikiliyor...\n";
//         return 1;
//     }

//     // YENİ API: Şirket adı, Template ID'si, isAI = true (Kendi kendine oynasın diye beynini takıyoruz!)
//     GameApi::SetPlayer("TRADEFALL AI INC.", "core_startup_company_001", true);
//     std::cout << "[INFO] Yapay Zeka sirketi masaya oturdu.\n";

//     ConsoleUtils::printConsole();
//     Console::clearLogs();

//     std::cout << "\n===================================================\n";
//     std::cout << "      HEADLESS SIMULASYON BASLIYOR (100 TUR)       \n";
//     std::cout << "===================================================\n\n";

//     // --- 4. HEADLESS GAME LOOP ---
//     // Simülasyonu şimdilik 100 tur çalıştırıp sonuçları görelim
//     for (int i = 1; i <= 100; ++i) {
        
//         GameApi::step(); // Motoru 1 tick ilerlet
        
//         // Konsolda biriken logları çek ve ekrana bas
//         auto logs = GameApi::readConsole();
//         for (const auto& logMsg : logs) {
//             std::cout << logMsg << "\n";
//         }

//         // Simülasyonun akışını gözle takip edebilmek için araya ufak bir gecikme koyuyoruz
//         // İstersen bunu silip 100 turu milisaniyeler içinde bitirmesini izleyebilirsin!
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }

//     std::cout << "\n===================================================\n";
//     std::cout << "             SIMULASYON TAMAMLANDI                 \n";
//     std::cout << "===================================================\n";

//     // İstersen test bittiğinde oyunun o anki tüm haritasını ve ekonomisini JSON olarak alıp kaydedebilirsin
//     // std::cout << GameApi::getSerializedState() << std::endl;\

//     try {
//       GameApi::saveGame("headless_test_save");
//     } catch (const std::exception& e) {
//       std::cerr << "[ERROR] Oyun kaydedilemedi: " << e.what() << std::endl;
//     }

//     ConsoleUtils::printConsole();

//     return 0;
// }

#include "webview.h"
#include "../Api/NativeApi.h"
#include "ModLoader.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

// HTML dosyasını okumak için basit bir yardımcı
std::string readHtml(const std::string& path) {
    std::ifstream f(path);
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

int main(int argc, char **argv) {
    std::cout << "[INFO] TradeFall Motoru baslatiliyor...\n";

    // --- Motor Başlatma ---
    GameApi::initEngine();
    ModLoader::loadMods("/home/efe/Documents/projects/project-augustus/mock/load_order.json");
    ModLoader::insertDataIntoEngine(GameApi::loadGameFiles);
    GameApi::startScenario("debug_scenario");
    GameApi::SetPlayer("Efe Ticaret A.Ş.", "core_startup_company_001", false);

    // --- WebView Kurulumu ---
    webview::webview w(true, nullptr); // true = debug modu (devtools açık)
    w.set_title("TradeFall - Project Augustus");
    w.set_size(1280, 720, WEBVIEW_HINT_NONE);

    // --- API Bağlantıları (JS -> C++) ---
    
    // 1. Oyunu ilerlet
    w.bind("api_step", [&](std::string req) -> std::string {
        GameApi::step();
        return "true";
    });

    // 2. Input Gönder (JS'den gelen stringleştirilmiş JSON'u motora iletir)
    w.bind("api_sendInput", [&](std::string req) -> std::string {
        try {
            auto args = nlohmann::json::parse(req);
            std::string payload = args[0].get<std::string>();
            bool success = GameApi::sendInput(payload);
            return success ? "true" : "false";
        } catch (...) {
            return "false";
        }
    });

    // 3. Konsolu Oku
    w.bind("api_readConsole", [&](std::string req) -> std::string {
        auto logs = GameApi::readConsole();
        nlohmann::json j = logs;
        return j.dump();
    });

    // 4. Durumu Çek
    w.bind("api_getSerializedState", [&](std::string req) -> std::string {
        return GameApi::getSerializedState();
    });

    // 5. Eventleri Çek
    w.bind("api_getPendingEvents", [&](std::string req) -> std::string {
        return GameApi::getPendingEvents();
    });

    // Arayüzü yükle ve döngüyü başlat
    std::string htmlContent = readHtml("/home/efe/Documents/projects/project-augustus/src/frontend/index.html");
    w.set_html(htmlContent);
    
    std::cout << "[INFO] UI Baslatildi. DevTools acik.\n";
    w.run();

    return 0;
}