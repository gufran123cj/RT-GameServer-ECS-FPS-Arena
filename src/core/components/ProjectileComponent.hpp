#pragma once

#include "../Component.hpp"
#include "../Entity.hpp"
#include <SFML/System/Vector2.hpp>

namespace game::core::components {

/**
 * Projectile Component
 * 
 * Stores information about a projectile entity.
 * Used for shooting mechanics - tracks owner, damage, direction, and speed.
 */
struct ProjectileComponent {
    game::EntityID ownerID;      // Entity ID of the player who shot this projectile
    float damage;                 // Damage amount (for future health system)
    float speed;                  // Projectile speed (pixels per second)
    sf::Vector2f direction;      // Normalized direction vector (from owner to target)
    
    ProjectileComponent() 
        : ownerID(game::INVALID_ENTITY)
        , damage(10.0f)
        , speed(300.0f)
        , direction(0.0f, 0.0f)
    {}
    
    ProjectileComponent(game::EntityID owner, float dmg, float spd, const sf::Vector2f& dir)
        : ownerID(owner)
        , damage(dmg)
        , speed(spd)
        , direction(dir)
    {}
};

} // namespace game::core::components

