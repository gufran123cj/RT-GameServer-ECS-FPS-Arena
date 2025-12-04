#pragma once

#include "../Component.hpp"

namespace game::core::components {

/**
 * Lifetime Component
 * 
 * Tracks the lifetime of an entity (typically projectiles).
 * Entity is destroyed when currentLifetime >= maxLifetime.
 */
struct LifetimeComponent {
    float maxLifetime;      // Maximum lifetime in seconds
    float currentLifetime;  // Current lifetime in seconds (incremented each frame)
    
    LifetimeComponent() 
        : maxLifetime(5.0f)
        , currentLifetime(0.0f)
    {}
    
    explicit LifetimeComponent(float maxLifetime)
        : maxLifetime(maxLifetime)
        , currentLifetime(0.0f)
    {}
    
    /**
     * Check if lifetime has expired
     */
    bool isExpired() const {
        return currentLifetime >= maxLifetime;
    }
};

} // namespace game::core::components

