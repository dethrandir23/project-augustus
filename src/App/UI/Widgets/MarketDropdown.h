// MarketDropdown.h
#include "Game/Gamestate.h"
#include "uuid/uuid.h"
#include <imgui.h>

namespace UI::Widgets {
    std::optional<uuids::uuid> MarketDropdown(Gamestate &gamestate);
}