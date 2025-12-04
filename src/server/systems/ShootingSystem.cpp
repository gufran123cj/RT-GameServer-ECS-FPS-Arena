#include "ShootingSystem.hpp"
#include "../../core/World.hpp"
#include "../../core/components/PositionComponent.hpp"
#include "../../core/components/VelocityComponent.hpp"
#include "../../core/components/SpriteComponent.hpp"
#include "../../core/components/ProjectileComponent.hpp"
#include "../../core/components/LifetimeComponent.hpp"
#include "../ServerNetworkManager.hpp"
#include "../../game/GameConstants.hpp"
#include <cmath>
#include <iostream>

namespace game::server::systems {

ShootingSystem::ShootingSystem(game::server::ServerNetworkManager& networkManager)
    : networkManager(networkManager) {
}

void ShootingSystem::update(float deltaTime, game::core::World& world) {
    // Get all shoot events from network manager
    auto shootEvents = networkManager.getShootEvents();
    
    for (const auto& [address, event] : shootEvents) {
        if (!event.valid) {
            continue;
        }
        
        // Get player entity for this address
        game::core::Entity playerEntity = networkManager.getClientEntity(address);
        if (!playerEntity.isValid()) {
            continue;  // Player not found
        }
        
        // Validate player ID matches
        if (playerEntity.id != event.playerID) {
            std::cerr << "Warning: Shoot event player ID mismatch for " << address.toString() << std::endl;
            continue;
        }
        
        // Get player position
        auto* playerPos = world.getComponent<game::core::components::PositionComponent>(playerEntity.id);
        if (!playerPos) {
            continue;  // Player has no position component
        }
        
        // Calculate direction from player to target
        sf::Vector2f direction = calculateDirection(playerPos->position, event.targetPosition);
        
        // Calculate spawn position (player position + offset in direction)
        sf::Vector2f spawnPosition = playerPos->position + direction * game::client::Constants::PROJECTILE_SPAWN_OFFSET;
        
        // Spawn projectile
        spawnProjectile(world, playerEntity.id, spawnPosition, direction);
    }
}

game::core::Entity ShootingSystem::spawnProjectile(
    game::core::World& world,
    game::EntityID ownerID,
    const sf::Vector2f& spawnPosition,
    const sf::Vector2f& direction) {
    
    // Create entity
    game::core::Entity projectile = world.createEntity();
    
    // Add PositionComponent
    game::core::components::PositionComponent posComp(spawnPosition);
    world.addComponent<game::core::components::PositionComponent>(projectile.id, posComp);
    
    // Add VelocityComponent (direction * speed)
    sf::Vector2f velocity = direction * game::client::Constants::PROJECTILE_SPEED;
    game::core::components::VelocityComponent velComp(velocity);
    world.addComponent<game::core::components::VelocityComponent>(projectile.id, velComp);
    
    // Add SpriteComponent (small yellow circle)
    game::core::components::SpriteComponent spriteComp(
        game::client::Constants::PROJECTILE_SIZE,
        game::client::Constants::PROJECTILE_COLOR
    );
    world.addComponent<game::core::components::SpriteComponent>(projectile.id, spriteComp);
    
    // Add ProjectileComponent
    game::core::components::ProjectileComponent projComp(
        ownerID,
        10.0f,  // damage
        game::client::Constants::PROJECTILE_SPEED,
        direction
    );
    world.addComponent<game::core::components::ProjectileComponent>(projectile.id, projComp);
    
    // Add LifetimeComponent
    game::core::components::LifetimeComponent lifetimeComp(game::client::Constants::PROJECTILE_LIFETIME);
    world.addComponent<game::core::components::LifetimeComponent>(projectile.id, lifetimeComp);
    
    return projectile;
}

sf::Vector2f ShootingSystem::calculateDirection(const sf::Vector2f& from, const sf::Vector2f& to) {
    sf::Vector2f dir = to - from;
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    
    // Normalize
    if (length > 0.0f) {
        dir.x /= length;
        dir.y /= length;
    } else {
        // Default direction (down) if positions are the same
        dir = sf::Vector2f(0.0f, 1.0f);
    }
    
    return dir;
}

} // namespace game::server::systems

