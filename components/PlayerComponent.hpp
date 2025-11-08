#pragma once

#include "../ecs/Component.hpp"
#include "../include/common/types.hpp"
#include "../net/Packet.hpp"
#include <string>
#include <cstddef>
#include <cstring>

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
    
    // Serialization (Phase 4)
    bool serialize(net::PacketWriter& writer) const override {
        if (!writer.write(playerID)) return false;
        if (!writer.write(rating)) return false;
        
        // Serialize string length and data
        uint16_t nameLen = static_cast<uint16_t>(name.length());
        if (!writer.write(nameLen)) return false;
        if (nameLen > 0) {
            return writer.writeBytes(name.c_str(), nameLen);
        }
        return true;
    }
    
    bool deserialize(net::PacketReader& reader) override {
        if (!reader.read(playerID)) return false;
        if (!reader.read(rating)) return false;
        
        // Deserialize string
        uint16_t nameLen = 0;
        if (!reader.read(nameLen)) return false;
        if (nameLen > 0) {
            name.resize(nameLen);
            if (!reader.readBytes(&name[0], nameLen)) return false;
        } else {
            name.clear();
        }
        return true;
    }
    
    size_t getSerializedSize() const override {
        return sizeof(PlayerID) + sizeof(float) + sizeof(uint16_t) + name.length();
    }
};

} // namespace game::components

