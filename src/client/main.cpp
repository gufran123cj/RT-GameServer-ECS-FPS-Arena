#include "ClientNetworkManager.hpp"
#include "../network/Packet.hpp"
#include "../network/PacketTypes.hpp"
#include "../core/components/PositionComponent.hpp"
#include "../core/components/SpriteComponent.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <map>

/**
 * Simple Test Client
 * 
 * Connects to server and logs connection status
 */
class TestClient : public game::client::ClientNetworkManager {
public:
    void onConnectAck(game::core::Entity::ID entityID) override {
        std::cout << "[CLIENT] Received CONNECT_ACK, Entity ID: " << entityID << std::endl;
    }
    
    void onSnapshot(game::network::Packet& packet) override {
        packet.resetRead();
        
        uint32_t entityCount;
        if (!packet.read(entityCount)) {
            return;
        }
        
        std::cout << "[CLIENT] Received SNAPSHOT with " << entityCount << " entities" << std::endl;
        
        // Read entities
        for (uint32_t i = 0; i < entityCount; ++i) {
            game::core::Entity::ID entityID;
            if (!packet.read(entityID)) break;
            
            float posX, posY;
            if (!packet.read(posX) || !packet.read(posY)) break;
            
            float sizeX, sizeY;
            if (!packet.read(sizeX) || !packet.read(sizeY)) break;
            
            uint8_t r, g, b, a;
            if (!packet.read(r) || !packet.read(g) || !packet.read(b) || !packet.read(a)) break;
            
            std::cout << "  Entity " << entityID << ": Position=(" << posX << ", " << posY 
                      << "), Size=(" << sizeX << ", " << sizeY << ")" << std::endl;
        }
    }
    
    void onDisconnect() override {
        std::cout << "[CLIENT] Disconnected from server" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::cout << "=== Test Client ===" << std::endl;
    
    // Get server address from command line or use default
    std::string serverIp = "127.0.0.1";
    uint16_t serverPort = 7777;
    
    if (argc >= 2) {
        serverIp = argv[1];
    }
    if (argc >= 3) {
        serverPort = static_cast<uint16_t>(std::stoi(argv[2]));
    }
    
    std::cout << "Connecting to server: " << serverIp << ":" << serverPort << std::endl;
    
    // Create client
    TestClient client;
    
    // Initialize network
    if (!client.initialize()) {
        std::cerr << "Failed to initialize client network" << std::endl;
        return 1;
    }
    
    // Connect to server
    if (!client.connect(serverIp, serverPort)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    // Main loop
    std::cout << "Client running... (Press Enter to disconnect)" << std::endl;
    
    auto lastHeartbeat = std::chrono::steady_clock::now();
    const float heartbeatInterval = 1.0f;  // 1 second
    
    while (client.isConnected() || !client.isConnected()) {  // Keep running even if not connected yet
        // Process incoming packets
        client.processPackets();
        
        // Send heartbeat periodically
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<float>(now - lastHeartbeat).count();
        
        if (elapsed >= heartbeatInterval && client.isConnected()) {
            game::network::Packet heartbeat(game::network::PacketType::HEARTBEAT);
            heartbeat.setSequence(1);
            heartbeat.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count()));
            
            client.sendPacket(heartbeat);
            lastHeartbeat = now;
        }
        
        // Check for user input (non-blocking)
        // Note: Windows doesn't have easy non-blocking stdin, so we'll just keep running
        // User can close with Ctrl+C
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Small sleep to prevent 100% CPU
    }
    
    // Disconnect
    client.disconnect();
    client.shutdown();
    
    std::cout << "Client stopped" << std::endl;
    return 0;
}

