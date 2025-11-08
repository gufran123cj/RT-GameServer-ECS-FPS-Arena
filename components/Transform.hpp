#pragma once

#include "../ecs/Component.hpp"
#include "../physics/Physics.hpp"
#include "../net/Packet.hpp"
#include <cstddef>

namespace game::components {

// Transform component - stores position, rotation, and scale
class Transform : public ecs::ComponentBase<Transform> {
public:
    physics::Vec3 position;
    physics::Vec3 rotation; // Euler angles in degrees (pitch, yaw, roll)
    physics::Vec3 scale;

    Transform() 
        : position(0.0f, 0.0f, 0.0f)
        , rotation(0.0f, 0.0f, 0.0f)
        , scale(1.0f, 1.0f, 1.0f) {}

    Transform(const physics::Vec3& pos) 
        : position(pos)
        , rotation(0.0f, 0.0f, 0.0f)
        , scale(1.0f, 1.0f, 1.0f) {}

    Transform(const physics::Vec3& pos, const physics::Vec3& rot) 
        : position(pos)
        , rotation(rot)
        , scale(1.0f, 1.0f, 1.0f) {}

    std::unique_ptr<ecs::Component> clone() const override {
        auto transform = std::make_unique<Transform>(position, rotation);
        transform->scale = scale;
        return transform;
    }
    
    // Serialization (Phase 4)
    bool serialize(net::PacketWriter& writer) const override {
        return writer.write(position.x) && writer.write(position.y) && writer.write(position.z) &&
               writer.write(rotation.x) && writer.write(rotation.y) && writer.write(rotation.z) &&
               writer.write(scale.x) && writer.write(scale.y) && writer.write(scale.z);
    }
    
    bool deserialize(net::PacketReader& reader) override {
        return reader.read(position.x) && reader.read(position.y) && reader.read(position.z) &&
               reader.read(rotation.x) && reader.read(rotation.y) && reader.read(rotation.z) &&
               reader.read(scale.x) && reader.read(scale.y) && reader.read(scale.z);
    }
    
    size_t getSerializedSize() const override {
        return sizeof(float) * 9; // position(3) + rotation(3) + scale(3)
    }
};

} // namespace game::components

