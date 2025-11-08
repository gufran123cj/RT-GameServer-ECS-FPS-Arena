#pragma once

#include "../ecs/Component.hpp"
#include "../include/common/types.hpp"
#include <string>

namespace game::components {

// PlayerComponent - links entity to a player
class PlayerComponent : public ecs::ComponentBase<PlayerComponent> {
public:
    PlayerID playerID;
    std::string name;
    float rating;

    PlayerComponent() : playerID(INVALID_PLAYER), name(""), rating(1000.0f) {}
    PlayerComponent(PlayerID id) : playerID(id), name(""), rating(1000.0f) {}
    PlayerComponent(PlayerID id, const std::string& playerName) 
        : playerID(id), name(playerName), rating(1000.0f) {}

    std::unique_ptr<ecs::Component> clone() const override {
        auto comp = std::make_unique<PlayerComponent>(playerID, name);
        comp->rating = rating;
        return comp;
    }
};

} // namespace game::components

