#include "GodotAugustus.h"
#include "Debug/MarketDebug.h"
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>

namespace godot {

GodotAugustus::GodotAugustus() {}
GodotAugustus::~GodotAugustus() {}

void GodotAugustus::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init"), &GodotAugustus::init);
    ClassDB::bind_method(D_METHOD("load_game_files", "contents", "names"), &GodotAugustus::load_game_files);
    ClassDB::bind_method(D_METHOD("load_data_directory", "path"), &GodotAugustus::load_data_directory);
    ClassDB::bind_method(D_METHOD("start_scenario", "scenario_id"), &GodotAugustus::start_scenario);
    ClassDB::bind_method(D_METHOD("set_player", "name", "template_id", "is_ai"), &GodotAugustus::set_player);

    ClassDB::bind_method(D_METHOD("step"), &GodotAugustus::step);
    ClassDB::bind_method(D_METHOD("step_n", "n"), &GodotAugustus::step_n);

    ClassDB::bind_method(D_METHOD("send_input", "json"), &GodotAugustus::send_input);

    ClassDB::bind_method(D_METHOD("get_serialized_state"), &GodotAugustus::get_serialized_state);
    ClassDB::bind_method(D_METHOD("get_delta_state"), &GodotAugustus::get_delta_state);
    ClassDB::bind_method(D_METHOD("get_player_state"), &GodotAugustus::get_player_state);
    ClassDB::bind_method(D_METHOD("get_market_data", "market_id"), &GodotAugustus::get_market_data);
    ClassDB::bind_method(D_METHOD("get_entity_orders", "owner_id"), &GodotAugustus::get_entity_orders);
    ClassDB::bind_method(D_METHOD("get_market_debug_stats", "scope", "id1", "id2"), &GodotAugustus::get_market_debug_stats);
    ClassDB::bind_method(D_METHOD("get_factory_status", "factory_id"), &GodotAugustus::get_factory_status);
    ClassDB::bind_method(D_METHOD("get_factory_templates"), &GodotAugustus::get_factory_templates);
    ClassDB::bind_method(D_METHOD("get_pending_events"), &GodotAugustus::get_pending_events);

    ClassDB::bind_method(D_METHOD("read_console"), &GodotAugustus::read_console);
    ClassDB::bind_method(D_METHOD("log_to_console", "msg"), &GodotAugustus::log_to_console);

    ClassDB::bind_method(D_METHOD("save_game", "name"), &GodotAugustus::save_game);
    ClassDB::bind_method(D_METHOD("load_game", "name"), &GodotAugustus::load_game);
    ClassDB::bind_method(D_METHOD("list_saves"), &GodotAugustus::list_saves);
}

void GodotAugustus::init() {
    augustus_engine::EngineController::instance().init();
}

bool GodotAugustus::load_game_files(const PackedStringArray &contents, const PackedStringArray &names) {
    std::vector<std::string> c, n;
    c.reserve(contents.size());
    n.reserve(names.size());
    for (int i = 0; i < contents.size(); i++)
        c.push_back(contents[i].utf8().get_data());
    for (int i = 0; i < names.size(); i++)
        n.push_back(names[i].utf8().get_data());
    return augustus_engine::EngineController::instance().loadGameFiles(c, n);
}

bool GodotAugustus::load_data_directory(const String &path) {
    PackedStringArray contents, names;
    std::vector<String> dirs;
    dirs.push_back(path);

    while (!dirs.empty()) {
        String current = dirs.back();
        dirs.pop_back();
        Ref<DirAccess> dir = DirAccess::open(current);
        if (dir.is_null()) continue;

        dir->list_dir_begin();
        String file = dir->get_next();
        while (!file.is_empty()) {
            String full = current.path_join(file);
            if (dir->current_is_dir()) {
                if (file != "." && file != "..") dirs.push_back(full);
            } else if (file.ends_with(".json")) {
                Ref<FileAccess> f = FileAccess::open(full, FileAccess::READ);
                if (f.is_valid()) {
                    contents.append(f->get_as_text());
                    names.append(file.trim_suffix(".json"));
                    f->close();
                }
            }
            file = dir->get_next();
        }
        dir->list_dir_end();
    }

    if (contents.is_empty()) return false;
    return load_game_files(contents, names);
}

bool GodotAugustus::start_scenario(const String &scenario_id) {
    return augustus_engine::EngineController::instance().startScenario(scenario_id.utf8().get_data());
}

void GodotAugustus::set_player(const String &name, const String &template_id, bool is_ai) {
    augustus_engine::EngineController::instance().setPlayer(
        name.utf8().get_data(), template_id.utf8().get_data(), is_ai);
}

void GodotAugustus::step() {
    augustus_engine::EngineController::instance().step();
}

void GodotAugustus::step_n(int n) {
    augustus_engine::EngineController::instance().stepN(static_cast<size_t>(n));
}

bool GodotAugustus::send_input(const String &input_text) {
    std::string raw_input = input_text.utf8().get_data();

    // 1. Önce doğrudan JSON formatında mı diye kontrol et (Motorun kendi iç sinyalleri vs. için)
    auto parsed = nlohmann::json::parse(raw_input, nullptr, false);

    // 2. Eğer JSON değilse (Yani kullanıcı konsola "help", "add_money 100" vs. yazdıysa)
    if (parsed.is_discarded()) {
        
        // Senin yazdığın CLI süzgecinden geçiriyoruz
        parsed = Console::parseInput(raw_input);

        // Eğer 'help' yazıldıysa veya hatalı komut girildiyse Console::parseInput() nullptr (veya null json) dönüyor.
        // Ama log'u C++ tarafında Console::logs içine çoktan ekledi.
        // Bu yüzden motorun (InputHandler) içine göndermemize gerek yok, işlemi bitir.
        if (parsed.is_null()) {
            return true; // İşlem aslında başarılı (log atıldı), sadece oyun state'ini değiştiren bir komut değil.
        }
    }

    // 3. Artık elimizde kesinlikle geçerli bir JSON komutu var {"type": "ADD_MONEY"...}
    // Bunu Augustus motorunun InputHandler'ına gönder!
    return augustus_engine::EngineController::instance().sendInput(parsed);
}

String GodotAugustus::get_serialized_state() {
    return String(augustus_engine::EngineController::instance().getSerializedState().c_str());
}

String GodotAugustus::get_delta_state() {
    return String(augustus_engine::EngineController::instance().getDeltaState().c_str());
}

String GodotAugustus::get_player_state() {
    return String(augustus_engine::EngineController::instance().getPlayerState().c_str());
}

String GodotAugustus::get_market_data(const String &market_id) {
    return String(augustus_engine::EngineController::instance().getMarketData(market_id.utf8().get_data()).c_str());
}

String GodotAugustus::get_entity_orders(const String &owner_id) {
    return String(augustus_engine::EngineController::instance().getEntityOrders(owner_id.utf8().get_data()).c_str());
}

static uuids::uuid parseUuidOrNil(const String &s) {
    std::string str = s.utf8().get_data();
    if (str.empty()) return uuids::uuid{};
    auto opt = uuids::uuid::from_string(str);
    return opt.has_value() ? opt.value() : uuids::uuid{};
}

String GodotAugustus::get_market_debug_stats(const String &scope, const String &id1, const String &id2) {
    auto &gs = augustus_engine::EngineController::instance().getGamestate();
    std::string s = scope.utf8().get_data();
    uuids::uuid id1u = parseUuidOrNil(id1);
    uuids::uuid id2u = parseUuidOrNil(id2);

    if (s == "global") {
        return String(MarketDebug::statsToJson(MarketDebug::getGlobalStats(gs)).dump().c_str());
    } else if (s == "market" && !id1u.is_nil()) {
        return String(MarketDebug::statsToJson(MarketDebug::getMarketStats(gs, id1u)).dump().c_str());
    } else if (s == "entity" && !id1u.is_nil()) {
        if (id2u.is_nil())
            return String(MarketDebug::statsToJson(MarketDebug::getEntityStats(gs, id1u)).dump().c_str());
        else
            return String(MarketDebug::statsToJson(MarketDebug::getEntityStatsInMarket(gs, id1u, id2u)).dump().c_str());
    } else if (s == "all") {
        return String(MarketDebug::getAllStatsAsJson(gs).dump().c_str());
    }
    return "{}";
}

String GodotAugustus::get_factory_status(const String &factory_id) {
    return String(augustus_engine::EngineController::instance().getFactoryStatus(factory_id.utf8().get_data()).c_str());
}

String GodotAugustus::get_factory_templates() {
    return String(augustus_engine::EngineController::instance().getFactoryTemplates().c_str());
}

String GodotAugustus::get_pending_events() {
    return String(augustus_engine::EngineController::instance().getPendingEvents().c_str());
}

PackedStringArray GodotAugustus::read_console() {
    auto logs = augustus_engine::EngineController::instance().readConsole();
    PackedStringArray result;
    for (const auto &log : logs)
        result.append(String(log.c_str()));
    return result;
}

void GodotAugustus::log_to_console(const String &msg) {
    augustus_engine::EngineController::instance().logToConsole(msg.utf8().get_data());
}

bool GodotAugustus::save_game(const String &name) {
    return augustus_engine::EngineController::instance().saveGame(name.utf8().get_data());
}

bool GodotAugustus::load_game(const String &name) {
    return augustus_engine::EngineController::instance().loadGame(name.utf8().get_data());
}

PackedStringArray GodotAugustus::list_saves() {
    auto saves = augustus_engine::EngineController::instance().listSaves();
    PackedStringArray result;
    for (const auto &s : saves)
        result.append(String(s.c_str()));
    return result;
}

}
