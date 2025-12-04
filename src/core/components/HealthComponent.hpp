#pragma once

#include "../Component.hpp"

namespace game::core::components {

/**
 * Health Component
 * 
 * Stores health information for an entity (typically players).
 * Health decreases when entity is hit by projectiles.
 */
struct HealthComponent {
    float maxHealth;      // Maximum health value
    float currentHealth;  // Current health value
    
    HealthComponent() 
        : maxHealth(10.0f)
        , currentHealth(10.0f)
    {}
    
    HealthComponent(float maxHealth)
        : maxHealth(maxHealth)
        , currentHealth(maxHealth)
    {}
    
    /**
     * Take damage
     * @param damage Amount of damage to take
     * @return True if entity is still alive after damage
     */
    bool takeDamage(float damage) {
        currentHealth -= damage;
        if (currentHealth < 0.0f) {
            currentHealth = 0.0f;
        }
        return currentHealth > 0.0f;
    }
    
    /**
     * Check if entity is alive
     */
    bool isAlive() const {
        return currentHealth > 0.0f;
    }
    
    /**
     * Check if entity is dead
     */
    bool isDead() const {
        return currentHealth <= 0.0f;
    }
    
    /**
     * Heal entity
     */
    void heal(float amount) {
        currentHealth += amount;
        if (currentHealth > maxHealth) {
            currentHealth = maxHealth;
        }
    }
};

} // namespace game::core::components

