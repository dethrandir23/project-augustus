// Console.cpp
#include "Console.h"
#include "../../lib/nlohmann/json.hpp"
#include <ctime>
#include <exception>
#include <stdexcept>
#include "CLI11/CLI11.hpp"

// FOR FUTURE: SEPERATE GAME AND DEBUG CONSOLE, RETURN DEBUG MESSAGES IF ONLY
// DEBUG MODE ENABLED OR ADD AN PARAMETER TO GET ONLY NOT DEBUG OR WITH DEBUG OR
// ONLY DEBUG LOGS

/*
        EXAMPLE INPUTS (CHEAT CODES)

-   add_money xxx
-   add_item item_id xxx
-   step_game xxx

*/

std::vector<Log> Console::logs;

std::string logTypeToStr(LogType type) {
  switch (type) {
  case LogType::INFO:
    return "INFO";
  case LogType::ERROR:
    return "ERROR";
  case LogType::WARNING:
    return "WARNING";
  default:
    return "UNKNOWN";
  }
}

LogType strToLogType(const std::string &str) {
  if (str == "INFO")
    return LogType::INFO;
  if (str == "ERROR")
    return LogType::ERROR;
  if (str == "WARNING") {
    return LogType::WARNING;
  }
  return LogType::INFO;
}

std::string Console::help() {
  // raw string
  std::string help = R"(
    Available commands are:
    add_money <amount>
    add_item <item_id> <amount>
    step_game <times>
    help
  )";
  return help;
}

void Console::logHelp() { Console::log(help(), LogType::INFO); }

nlohmann::json Console::parseInput(const std::string &input) {
    CLI::App app{"Game Console"};
    app.name(""); // Help çıktısında temiz görünsün
    
    nlohmann::json result; 
    bool command_found = false;

    // --- DEĞİŞKENLER (Komutlardan gelen verileri tutacaklar) ---
    // Bu değişkenler lambda capture ile dolacak
    std::string str_arg1, str_arg2, str_arg3; 
    double d_arg1 = 0;
    float f_arg1 = 0;
    int i_arg1 = 0;
    
    // Opsiyonel ID'ler için (Boşsa gönderilmeyecek mantığı için)
    std::string opt_company_id = "";
    std::string opt_owner_id = "";
    std::string opt_name = "";

    // ---------------------------------------------------------
    // 1. STEP GAME
    // ---------------------------------------------------------
    auto* sub_step = app.add_subcommand("step", "Simulasyonu ilerletir");
    sub_step->add_option("times", i_arg1, "Kac tick ilerlesin")->default_val(1);
    sub_step->callback([&]() {
        result["type"] = "STEP_GAME";
        result["payload"]["times"] = i_arg1;
        command_found = true;
    });

    // ---------------------------------------------------------
    // 2. BUILD FACTORY (build -t template_id [-c company_id] [-n name])
    // ---------------------------------------------------------
    auto* sub_build = app.add_subcommand("build_factory", "Yeni fabrika kurar");
    sub_build->add_option("-t,--template", str_arg1, "Template ID")->required();
    sub_build->add_option("-c,--company", opt_company_id, "Target Company ID (Opsiyonel)");
    sub_build->add_option("-n,--name", opt_name, "Custom Name (Opsiyonel)");
    sub_build->callback([&]() {
        result["type"] = "BUILD_FACTORY";
        result["payload"]["templateId"] = str_arg1;
        if(!opt_company_id.empty()) result["payload"]["companyId"] = opt_company_id;
        if(!opt_name.empty())       result["payload"]["customName"] = opt_name;
        command_found = true;
    });

    // ---------------------------------------------------------
    // 3. SCRAP FACTORY (scrap -f factory_id)
    // ---------------------------------------------------------
    auto* sub_scrap = app.add_subcommand("scrap_factory", "Fabrikayi yikar");
    sub_scrap->add_option("-f,--factory", str_arg1, "Factory ID")->required();
    sub_scrap->callback([&]() {
        result["type"] = "SCRAP_FACTORY";
        result["payload"]["factoryId"] = str_arg1;
        command_found = true;
    });

    // ---------------------------------------------------------
    // 4. RENAME FACTORY (rename -f factory_id -n new_name)
    // ---------------------------------------------------------
    auto* sub_rename = app.add_subcommand("rename_factory", "Fabrika ismini degistirir");
    sub_rename->add_option("-f,--factory", str_arg1, "Factory ID")->required();
    sub_rename->add_option("-n,--name", str_arg2, "New Name")->required();
    sub_rename->callback([&]() {
        result["type"] = "RENAME_FACTORY";
        result["payload"]["factoryId"] = str_arg1;
        result["payload"]["newName"] = str_arg2;
        command_found = true;
    });

    // ---------------------------------------------------------
    // 5. CHANGE WORKFORCE (workforce -f factory_id -d delta)
    // ---------------------------------------------------------
    auto* sub_work = app.add_subcommand("workforce", "Isci sayisini degistirir");
    sub_work->add_option("-f,--factory", str_arg1, "Factory ID")->required();
    sub_work->add_option("-d,--delta", i_arg1, "Degisim (Pozitif ekler, negatif cikarir)")->required();
    sub_work->callback([&]() {
        result["type"] = "CHANGE_WORKFORCE";
        result["payload"]["factoryId"] = str_arg1;
        result["payload"]["delta"] = i_arg1;
        command_found = true;
    });

    // ---------------------------------------------------------
    // 6. MARKET BUY (buy -m market_id -i item_id -a amount -p price [-o owner_id])
    // ---------------------------------------------------------
    auto* sub_buy = app.add_subcommand("buy", "Marketten urun alir");
    sub_buy->add_option("-m,--market", str_arg1, "Market ID")->required();
    sub_buy->add_option("-i,--item", str_arg2, "Item ID")->required();
    sub_buy->add_option("-a,--amount", f_arg1, "Miktar")->required();
    sub_buy->add_option("-p,--price", d_arg1, "Fiyat Limiti")->required();
    sub_buy->add_option("-o,--owner", opt_owner_id, "Buyer Company ID (Opsiyonel)");
    sub_buy->callback([&]() {
        result["type"] = "MARKET_BUY_ITEM";
        result["payload"] = {
            {"marketId", str_arg1}, {"itemId", str_arg2},
            {"amount", f_arg1}, {"price", d_arg1}
        };
        if(!opt_owner_id.empty()) result["payload"]["ownerId"] = opt_owner_id;
        command_found = true;
    });

    // ---------------------------------------------------------
    // 7. MARKET SELL (sell -m market_id -i item_id -a amount -p price [-o owner_id])
    // ---------------------------------------------------------
    auto* sub_sell = app.add_subcommand("sell", "Markete urun satar");
    sub_sell->add_option("-m,--market", str_arg1, "Market ID")->required();
    sub_sell->add_option("-i,--item", str_arg2, "Item ID")->required();
    sub_sell->add_option("-a,--amount", f_arg1, "Miktar")->required();
    sub_sell->add_option("-p,--price", d_arg1, "Fiyat Limiti")->required();
    sub_sell->add_option("-o,--owner", opt_owner_id, "Seller Company ID (Opsiyonel)");
    sub_sell->callback([&]() {
        result["type"] = "MARKET_SELL_ITEM";
        result["payload"] = {
            {"marketId", str_arg1}, {"itemId", str_arg2},
            {"amount", f_arg1}, {"price", d_arg1}
        };
        if(!opt_owner_id.empty()) result["payload"]["ownerId"] = opt_owner_id;
        command_found = true;
    });

    // ---------------------------------------------------------
    // 8. ADD MONEY (add_money amount [-c company_id])
    // ---------------------------------------------------------
    auto* sub_money = app.add_subcommand("add_money", "Para ekler (Hile)");
    sub_money->add_option("amount", d_arg1, "Miktar")->required();
    sub_money->add_option("-c,--company", opt_company_id, "Target Company ID (Opsiyonel)");
    sub_money->callback([&]() {
        result["type"] = "ADD_MONEY";
        result["payload"]["amount"] = d_arg1;
        if(!opt_company_id.empty()) result["payload"]["companyId"] = opt_company_id;
        command_found = true;
    });

    // ---------------------------------------------------------
    // 9. ADD ITEM (add_item item_id amount [-c company_id])
    // ---------------------------------------------------------
    auto* sub_additem = app.add_subcommand("add_item", "Envantere item ekler (Hile)");
    sub_additem->add_option("item_id", str_arg1, "Item ID")->required();
    sub_additem->add_option("amount", f_arg1, "Miktar")->required();
    sub_additem->add_option("-c,--company", opt_company_id, "Target Company ID (Opsiyonel)");
    sub_additem->callback([&]() {
        result["type"] = "ADD_ITEM";
        result["payload"] = { {"itemId", str_arg1}, {"amount", f_arg1} };
        if(!opt_company_id.empty()) result["payload"]["companyId"] = opt_company_id;
        command_found = true;
    });

    auto* sub_clear = app.add_subcommand("clear", "Konsol temizler");
    sub_clear->callback([&]() {
        Console::clearLogs();
        command_found = true;
    });


    // ---------------------------------------------------------
    // HELP ve PARSE
    // ---------------------------------------------------------
    app.set_help_flag("-h,--help", "Yardim goster");

    try {
        app.parse(input);
    } catch (const CLI::CallForHelp &e) {
        Console::log(app.help());
        return nullptr;
    } catch (const CLI::ParseError &e) {
        Console::log("Hatali komut: " + std::string(e.what()), LogType::ERROR);
        Console::log("Komut listesi icin '-h' veya '--help' kullanin.", LogType::INFO);
        return nullptr;
    }

    return command_found ? result : nullptr;
}

void Console::log(const std::string &s) {
  logs.push_back(Log(s, LogType::INFO));
}
void Console::log(const std::string &s, LogType type) {
  logs.push_back(Log(s, type));
}
void Console::log(Log log) { logs.push_back(log); }

std::string Console::readLog(size_t index) {
  if (index < logs.size()) {
    return logs[index].message;
  }
  return "";
}
Log Console::getLog(size_t index) {
  if (index < logs.size()) {
    return logs[index];
  }
  return Log();
}

void Console::clearLogs() { logs.clear(); }
void Console::removeLog(size_t index) {
  if (index < logs.size()) {
    logs.erase(logs.begin() + index);
  }
}
size_t Console::getLogCount() { return logs.size(); }

std::vector<Log> Console::getLogs() { return logs; }
std::vector<std::string> Console::getLogMessages() {
  std::vector<std::string> messages;
  for (const auto &log : logs) {
    messages.push_back(log.message);
  }

  return messages;
}