#pragma once

#include "../ecs/Component.hpp"
#include "../physics/Physics.hpp"
#include "../net/Packet.hpp"
#include <cstddef>

namespace game::components {

// Position component - stores entity position in 3D space
class Position : public ecs::ComponentBase<Position> {
public:
    physics::Vec3 value;

    Position() : value(0.0f, 0.0f, 0.0f) {}
    Position(float x, float y, float z) : value(x, y, z) {}
    Position(const physics::Vec3& pos) : value(pos) {}

    std::unique_ptr<ecs::Component> clone() const override {
        return std::make_unique<Position>(value);
    }
    
    // Serialization (Phase 4)
    bool serialize(net::PacketWriter& writer) const override {
        return writer.write(value.x) && writer.write(value.y) && writer.write(value.z);
    }
    
    bool deserialize(net::PacketReader& reader) override {
        return reader.read(value.x) && reader.read(value.y) && reader.read(value.z);
    }
    
    size_t getSerializedSize() const override {
        return sizeof(float) * 3; // x, y, z
    }
};

} // namespace game::components

