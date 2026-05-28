#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "Api/EngineController.h"

namespace godot {

class GodotAugustus : public Node {
    GDCLASS(GodotAugustus, Node)

protected:
    static void _bind_methods();

public:
    GodotAugustus();
    ~GodotAugustus();

    // Init & Data Loading
    void init();
    bool load_game_files(const PackedStringArray &contents, const PackedStringArray &names);
    bool load_data_directory(const String &path);
    bool start_scenario(const String &scenario_id);
    void set_player(const String &name, const String &template_id, bool is_ai);

    // Game Loop
    void step();
    void step_n(int n);

    // Input
    bool send_input(const String &json);

    // State Queries
    String get_serialized_state();
    String get_delta_state();
    String get_player_state();
    String get_market_data(const String &market_id);
    String get_factory_status(const String &factory_id);
    String get_factory_templates();
    String get_pending_events();

    // Console
    PackedStringArray read_console();
    void log_to_console(const String &msg);

    // Save / Load
    bool save_game(const String &name);
    bool load_game(const String &name);
    PackedStringArray list_saves();
};

}
