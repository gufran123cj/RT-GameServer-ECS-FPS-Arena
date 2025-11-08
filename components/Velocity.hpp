#pragma once

#include "../ecs/Component.hpp"
#include "../physics/Physics.hpp"

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
};

} // namespace game::components

