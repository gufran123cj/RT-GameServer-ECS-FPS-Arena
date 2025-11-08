#pragma once

#include "../ecs/Component.hpp"
#include "../physics/Physics.hpp"

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
};

} // namespace game::components

