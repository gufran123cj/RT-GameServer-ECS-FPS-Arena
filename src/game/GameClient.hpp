#pragma once

#include <map>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../client/ClientNetworkManager.hpp"
#include "../network/Packet.hpp"

namespace game::client {

/**
 * Game Client
 * 
 * Network client wrapper for game with entity tracking
 */
class GameClient : public game::client::ClientNetworkManager {
public:
    void onConnectAck(game::core::Entity::ID entityID) override;
    void onSnapshot(game::network::Packet& packet) override;
    void onDisconnect() override;
    
    game::core::Entity::ID myEntityID = 0;
    
    struct RemoteEntity {
        sf::Vector2f position;
        sf::Vector2f size;
        sf::Color color;
        float health = 10.0f;      // Current health
        float maxHealth = 10.0f;  // Maximum health
        bool hasHealth = false;   // Whether entity has health component
    };
    
    std::map<game::core::Entity::ID, RemoteEntity> remoteEntities;
};

} // namespace game::client

