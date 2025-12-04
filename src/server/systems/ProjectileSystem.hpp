#pragma once

#include "../../core/System.hpp"
#include "../../core/Entity.hpp"
#include "../CollisionHelper.hpp"
#include <vector>
#include <SFML/Graphics/Rect.hpp>

namespace game::core {
    class World;
}

namespace game::server::systems {

/**
 * Projectile System
 * 
 * Handles projectile lifetime, collision detection, and cleanup.
 * Runs after CollisionSystem but before MovementSystem.
 * 
 * Priority: 75 (runs after CollisionSystem (50), before MovementSystem (100))
 */
class ProjectileSystem : public game::core::System {
public:
    /**
     * Constructor
     * @param colliders List of static colliders (walls, obstacles)
     */
    explicit ProjectileSystem(const std::vector<sf::FloatRect>& colliders);
    
    ~ProjectileSystem() override = default;
    
    /**
     * Update projectile system
     * Updates lifetime, checks collisions, and destroys expired/collided projectiles
     */
    void update(float deltaTime, game::core::World& world) override;
    
    /**
     * Get system priority (lower = earlier execution)
     */
    int getPriority() const override {
        return 75;  // After CollisionSystem (50), before MovementSystem (100)
    }
    
    /**
     * Set colliders (for dynamic map loading)
     */
    void setColliders(const std::vector<sf::FloatRect>& colliders) {
        this->colliders = colliders;
    }

private:
    std::vector<sf::FloatRect> colliders;
    
    /**
     * Check if projectile should be destroyed (collision or timeout)
     * @param entityID Projectile entity ID
     * @param world ECS world reference
     * @param deltaTime Time step
     * @return True if projectile should be destroyed
     */
    bool shouldDestroyProjectile(game::core::Entity::ID entityID, game::core::World& world, float deltaTime);
};

} // namespace game::server::systems

