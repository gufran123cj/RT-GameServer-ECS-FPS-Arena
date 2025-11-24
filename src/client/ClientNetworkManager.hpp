#pragma once

#include <SFML/Network/UdpSocket.hpp>
#include <SFML/System/Vector2.hpp>
#include "../network/Address.hpp"
#include "../network/Packet.hpp"
#include "../core/Entity.hpp"

namespace game::client {

/**
 * Client Network Manager
 * 
 * UDP socket yönetimi, server'a bağlanma, packet gönderme/alma
 */
class ClientNetworkManager {
public:
    ClientNetworkManager();
    ~ClientNetworkManager();
    
    // Non-copyable
    ClientNetworkManager(const ClientNetworkManager&) = delete;
    ClientNetworkManager& operator=(const ClientNetworkManager&) = delete;
    
    // Movable
    ClientNetworkManager(ClientNetworkManager&&) noexcept = default;
    ClientNetworkManager& operator=(ClientNetworkManager&&) noexcept = default;
    
    /**
     * Initialize network (bind to any available port)
     */
    bool initialize();
    
    /**
     * Shutdown network
     */
    void shutdown();
    
    /**
     * Connect to server
     * @param initialPosition Optional initial position to send to server
     */
    bool connect(const std::string& serverIp, uint16_t serverPort, const sf::Vector2f& initialPosition = sf::Vector2f(0, 0));
    
    /**
     * Disconnect from server
     */
    void disconnect();
    
    /**
     * Process incoming packets
     * Returns number of packets processed
     */
    int processPackets();
    
    /**
     * Send packet to server
     */
    bool sendPacket(const game::network::Packet& packet);
    
    /**
     * Check if connected
     */
    bool isConnected() const { return connected; }
    
    /**
     * Get assigned entity ID from server
     */
    game::core::Entity::ID getEntityID() const { return entityID; }
    
    /**
     * Get server address
     */
    const game::network::Address& getServerAddress() const { return serverAddress; }
    
    /**
     * Callback for received packets (override in derived class or use function pointer)
     */
    virtual void onConnectAck(game::core::Entity::ID entityID) {}
    virtual void onSnapshot(game::network::Packet& packet) {}  // Non-const for reading
    virtual void onDisconnect() {}
    
private:
    sf::UdpSocket socket;
    game::network::Address serverAddress;
    bool connected;
    game::core::Entity::ID entityID;
    uint32_t nextSequenceNumber;
    
    /**
     * Handle incoming packet
     */
    void handlePacket(const game::network::Packet& packet);
};

} // namespace game::client

