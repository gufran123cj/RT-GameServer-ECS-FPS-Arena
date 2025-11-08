#pragma once

#include "../ecs/Component.hpp"
#include "../physics/Physics.hpp"

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
};

} // namespace game::components

