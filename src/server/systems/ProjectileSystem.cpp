#include "ProjectileSystem.hpp"
#include "../../core/World.hpp"
#include "../../core/components/PositionComponent.hpp"
#include "../../core/components/VelocityComponent.hpp"
#include "../../core/components/SpriteComponent.hpp"
#include "../../core/components/ProjectileComponent.hpp"
#include "../../core/components/LifetimeComponent.hpp"
#include "../../core/components/HealthComponent.hpp"
#include "../../core/components/KillCounterComponent.hpp"
#include "../CollisionHelper.hpp"
#include <iostream>

namespace game::server::systems {

ProjectileSystem::ProjectileSystem(const std::vector<sf::FloatRect>& colliders)
    : colliders(colliders) {
}

void ProjectileSystem::update(float deltaTime, game::core::World& world) {
    // Get all entities with projectile components
    auto projectiles = world.getEntitiesWith<
        game::core::components::PositionComponent,
        game::core::components::VelocityComponent,
        game::core::components::SpriteComponent,
        game::core::components::ProjectileComponent,
        game::core::components::LifetimeComponent
    >();
    
    // Check each projectile
    std::vector<game::core::Entity::ID> toDestroy;
    
    for (game::core::Entity::ID id : projectiles) {
        if (shouldDestroyProjectile(id, world, deltaTime)) {
            toDestroy.push_back(id);
        }
    }
    
    // Destroy projectiles that should be removed
    for (game::core::Entity::ID id : toDestroy) {
        // Create Entity with ID and generation 0 (generation is managed by World)
        game::core::Entity entity(id, 0);
        world.destroyEntity(entity);
    }
}

bool ProjectileSystem::shouldDestroyProjectile(
    game::core::Entity::ID entityID,
    game::core::World& world,
    float deltaTime) {
    
    // Get components
    auto* lifetime = world.getComponent<game::core::components::LifetimeComponent>(entityID);
    auto* pos = world.getComponent<game::core::components::PositionComponent>(entityID);
    auto* sprite = world.getComponent<game::core::components::SpriteComponent>(entityID);
    
    if (!lifetime || !pos || !sprite) {
        return true;  // Missing required components, destroy
    }
    
    // Update lifetime
    lifetime->currentLifetime += deltaTime;
    
    // Check timeout
    if (lifetime->isExpired()) {
        return true;  // Lifetime expired
    }
    
    // Check wall collision
    if (CollisionHelper::wouldCollideAt(pos->position, sprite->size, colliders)) {
        return true;  // Collision with wall
    }
    
    // Check player collision (damage system)
    auto* projComp = world.getComponent<game::core::components::ProjectileComponent>(entityID);
    if (projComp) {
        // Get all players (entities with HealthComponent)
        auto players = world.getEntitiesWith<
            game::core::components::PositionComponent,
            game::core::components::SpriteComponent,
            game::core::components::HealthComponent
        >();
        
        for (game::core::Entity::ID playerID : players) {
            // Skip if projectile owner is the same as player (can't hit yourself)
            if (playerID == projComp->ownerID) {
                continue;
            }
            
            // Check collision with player
            auto* playerPos = world.getComponent<game::core::components::PositionComponent>(playerID);
            auto* playerSprite = world.getComponent<game::core::components::SpriteComponent>(playerID);
            auto* playerHealth = world.getComponent<game::core::components::HealthComponent>(playerID);
            
            if (!playerPos || !playerSprite || !playerHealth) {
                continue;
            }
            
            // Check if projectile collides with player
            // Use CollisionHelper to get proper player collider (bottom half)
            sf::FloatRect playerCollider = CollisionHelper::getPlayerCollider(playerPos->position, playerSprite->size);
            
            // Projectile rect (simple AABB, origin at top-left)
            sf::FloatRect projRect(pos->position.x, pos->position.y, sprite->size.x, sprite->size.y);
            
            if (projRect.intersects(playerCollider)) {
                // Hit player! Apply damage
                bool stillAlive = playerHealth->takeDamage(projComp->damage);
                
                std::cout << "Player " << playerID << " hit! Health: " << playerHealth->currentHealth << "/" << playerHealth->maxHealth << std::endl;
                
                // If player is dead, give kill to projectile owner
                if (!stillAlive) {
                    std::cout << "Player " << playerID << " is dead! (Health: 0)" << std::endl;
                    
                    // Give kill to projectile owner
                    auto* ownerKillCounter = world.getComponent<game::core::components::KillCounterComponent>(projComp->ownerID);
                    if (ownerKillCounter) {
                        ownerKillCounter->addKill();
                        std::cout << "Player " << projComp->ownerID << " got a kill! Total kills: " << ownerKillCounter->getKills() << std::endl;
                    } else {
                        // Add KillCounterComponent if it doesn't exist
                        game::core::components::KillCounterComponent killCounter;
                        killCounter.addKill();
                        world.addComponent<game::core::components::KillCounterComponent>(projComp->ownerID, killCounter);
                        std::cout << "Player " << projComp->ownerID << " got a kill! Total kills: 1" << std::endl;
                    }
                }
                
                return true;  // Destroy projectile after hitting player
            }
        }
    }
    
    return false;  // Keep projectile alive
}

} // namespace game::server::systems

