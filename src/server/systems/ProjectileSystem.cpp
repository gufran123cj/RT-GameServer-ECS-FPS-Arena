#include "ProjectileSystem.hpp"
#include "../../core/World.hpp"
#include "../../core/components/PositionComponent.hpp"
#include "../../core/components/VelocityComponent.hpp"
#include "../../core/components/SpriteComponent.hpp"
#include "../../core/components/ProjectileComponent.hpp"
#include "../../core/components/LifetimeComponent.hpp"
#include "../CollisionHelper.hpp"

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
    
    // TODO: Check player collision (for future damage system)
    // For now, projectiles only collide with walls
    
    return false;  // Keep projectile alive
}

} // namespace game::server::systems

