#include "GameClient.hpp"
#include <SFML/Graphics.hpp>

namespace game::client {

void GameClient::onConnectAck(game::core::Entity::ID entityID) {
    myEntityID = entityID;
}

void GameClient::onSnapshot(game::network::Packet& packet) {
    packet.resetRead();
    
    uint32_t entityCount;
    if (!packet.read(entityCount)) {
        return;
    }
    
    remoteEntities.clear();
    
    for (uint32_t i = 0; i < entityCount; ++i) {
        game::core::Entity::ID entityID;
        if (!packet.read(entityID)) break;
        
        float posX, posY;
        if (!packet.read(posX) || !packet.read(posY)) break;
        
        float sizeX, sizeY;
        if (!packet.read(sizeX) || !packet.read(sizeY)) break;
        
        uint8_t r, g, b, a;
        if (!packet.read(r) || !packet.read(g) || !packet.read(b) || !packet.read(a)) break;
        
        RemoteEntity entity;
        entity.position = sf::Vector2f(posX, posY);
        entity.size = sf::Vector2f(sizeX, sizeY);
        entity.color = sf::Color(r, g, b, a);
        
        // Read HealthComponent (if exists)
        uint8_t hasHealth = 0;
        if (packet.read(hasHealth) && hasHealth == 1) {
            float currentHealth, maxHealth;
            if (packet.read(currentHealth) && packet.read(maxHealth)) {
                entity.health = currentHealth;
                entity.maxHealth = maxHealth;
                entity.hasHealth = true;
            }
        }
        
        remoteEntities[entityID] = entity;
    }
}

void GameClient::onDisconnect() {
    remoteEntities.clear();
    myEntityID = 0;
    std::cout << "Disconnected from server (player died or server disconnected)" << std::endl;
}

} // namespace game::client

