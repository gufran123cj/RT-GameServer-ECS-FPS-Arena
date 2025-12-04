#pragma once

#include "../../core/System.hpp"
#include "../../core/Entity.hpp"
#include <vector>
#include <SFML/System/Vector2.hpp>

namespace game::core {
    class World;
}

namespace game::server {
    class ServerNetworkManager;
}

namespace game::server::systems {

/**
 * Shooting System
 * 
 * Handles shooting input from clients and spawns projectile entities.
 * Runs early in the update cycle (priority 10).
 * 
 * Priority: 10 (runs after input processing, before movement)
 */
class ShootingSystem : public game::core::System {
public:
    /**
     * Constructor
     * @param networkManager Reference to network manager (for receiving SHOOT packets)
     */
    explicit ShootingSystem(game::server::ServerNetworkManager& networkManager);
    
    ~ShootingSystem() override = default;
    
    /**
     * Update shooting system
     * Processes SHOOT packets and spawns projectiles
     */
    void update(float deltaTime, game::core::World& world) override;
    
    /**
     * Get system priority (lower = earlier execution)
     */
    int getPriority() const override {
        return 10;  // Early in update cycle
    }

private:
    game::server::ServerNetworkManager& networkManager;
    
    /**
     * Spawn a projectile entity
     * @param world ECS world reference
     * @param ownerID Entity ID of the player who shot
     * @param spawnPosition Spawn position (player position + offset)
     * @param direction Normalized direction vector
     * @return Created projectile entity
     */
    game::core::Entity spawnProjectile(
        game::core::World& world,
        game::EntityID ownerID,
        const sf::Vector2f& spawnPosition,
        const sf::Vector2f& direction
    );
    
    /**
     * Calculate normalized direction vector
     */
    static sf::Vector2f calculateDirection(const sf::Vector2f& from, const sf::Vector2f& to);
};

} // namespace game::server::systems

