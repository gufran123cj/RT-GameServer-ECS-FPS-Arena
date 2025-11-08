#pragma once

#include "../ecs/System.hpp"
#include "../components/InputComponent.hpp"
#include "../components/Velocity.hpp"
#include "../components/Position.hpp"
#include "../components/Transform.hpp"
#include "../physics/Physics.hpp"
#include <cmath>

namespace game::systems {

// Movement System - Converts input to velocity and updates position
class MovementSystem : public ecs::SystemBase<components::InputComponent, components::Velocity, components::Position, components::Transform> {
private:
    const float MOVE_SPEED = 5.0f;        // Units per second
    const float SPRINT_MULTIPLIER = 1.5f; // Sprint speed multiplier
    const float MAX_SPEED = 10.0f;        // Maximum movement speed

public:
    MovementSystem() = default;
    
    int getPriority() const override {
        return 10; // Early priority - movement should happen before other systems
    }
    
    void process(ecs::World& world, float deltaTime, ecs::Entity& entity, 
                 components::InputComponent& input, 
                 components::Velocity& velocity,
                 components::Position& position,
                 components::Transform& transform) override {
        
        // Convert input flags to movement direction
        physics::Vec3 moveDirection(0.0f, 0.0f, 0.0f);
        
        // Forward/Backward (W/S)
        if (input.isPressed(components::INPUT_FORWARD)) {
            moveDirection.z += 1.0f;
        }
        if (input.isPressed(components::INPUT_BACKWARD)) {
            moveDirection.z -= 1.0f;
        }
        
        // Left/Right (A/D) - strafe
        if (input.isPressed(components::INPUT_LEFT)) {
            moveDirection.x -= 1.0f;
        }
        if (input.isPressed(components::INPUT_RIGHT)) {
            moveDirection.x += 1.0f;
        }
        
        // Normalize movement direction
        if (moveDirection.lengthSq() > 0.0f) {
            moveDirection = moveDirection.normalized();
            
            // Apply rotation (yaw) to movement direction
            float yawRad = input.mouseYaw * (3.14159f / 180.0f); // Convert to radians
            float cosYaw = cosf(yawRad);
            float sinYaw = sinf(yawRad);
            
            // Rotate movement vector by yaw
            float newX = moveDirection.x * cosYaw - moveDirection.z * sinYaw;
            float newZ = moveDirection.x * sinYaw + moveDirection.z * cosYaw;
            moveDirection.x = newX;
            moveDirection.z = newZ;
            
            // Calculate speed (with sprint multiplier)
            float speed = MOVE_SPEED;
            if (input.isPressed(components::INPUT_SPRINT)) {
                speed *= SPRINT_MULTIPLIER;
            }
            
            // Set velocity
            velocity.value = moveDirection * speed;
        } else {
            // No input - apply friction (gradual stop)
            velocity.value = velocity.value * 0.8f; // Friction factor
            if (velocity.value.lengthSq() < 0.01f) {
                velocity.value = physics::Vec3(0.0f, 0.0f, 0.0f);
            }
        }
        
        // Clamp velocity to max speed
        float currentSpeed = velocity.value.length();
        if (currentSpeed > MAX_SPEED) {
            velocity.value = velocity.value.normalized() * MAX_SPEED;
        }
        
        // Update position based on velocity
        position.value = position.value + (velocity.value * deltaTime);
        
        // Update transform component to match position
        transform.position = position.value;
        transform.rotation.y = input.mouseYaw; // Update rotation from input
    }
};

} // namespace game::systems

