#pragma once

#include <iostream>
#include "../System.hpp"
#include "../World.hpp"
#include "../components/PositionComponent.hpp"
#include "../components/VelocityComponent.hpp"

namespace game::core::systems {

/**
 * Movement System
 * 
 * Updates PositionComponent based on VelocityComponent.
 * Formula: position += velocity * deltaTime
 * 
 * This system processes all entities that have both
 * PositionComponent and VelocityComponent.
 */
class MovementSystem : public System {
public:
    MovementSystem() = default;
    ~MovementSystem() override = default;
    
    void update(float deltaTime, World& world) override {
        auto& positions = world.getStorage<components::PositionComponent>();
        auto& velocities = world.getStorage<components::VelocityComponent>();
        
        for (const auto& pair : velocities) {
            Entity::ID entityID = pair.first;
            const auto& velocity = *pair.second;
            auto* position = positions.get(entityID);
            if (position) {
                std::cout << "Movement for playerId: " << entityID << std::endl;
                position->position += velocity.velocity * deltaTime;
            }
        }
    }
    
    int getPriority() const override {
        return 100; // Movement happens early in the update cycle
    }
};

} // namespace game::core::systems

