#pragma once

#include "../ecs/Component.hpp"
#include "../physics/Physics.hpp"
#include "../net/Packet.hpp"
#include <cstddef>

namespace game::components {

// Velocity component - stores entity velocity in 3D space
class Velocity : public ecs::ComponentBase<Velocity> {
public:
    physics::Vec3 value;

    Velocity() : value(0.0f, 0.0f, 0.0f) {}
    Velocity(float x, float y, float z) : value(x, y, z) {}
    Velocity(const physics::Vec3& vel) : value(vel) {}

    std::unique_ptr<ecs::Component> clone() const override {
        return std::make_unique<Velocity>(value);
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

